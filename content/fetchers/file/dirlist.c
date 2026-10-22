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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/nscolour.h"

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "netsurf/plot_style.h"

#অন্তর্ভাব "dirlist.h"
#অন্তর্ভাব "desktop/system_colour.h"

স্থির পূর্ণসংখ্যা dirlist_filesize_calculate(unsigned long *bytesize);
স্থির পূর্ণসংখ্যা dirlist_filesize_মান(unsigned long bytesize);
স্থির অক্ষর* dirlist_filesize_unit(unsigned long bytesize);


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

bool dirlist_generate_শীর্ষ(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	পূর্ণসংখ্যা error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;

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

bool dirlist_generate_hide_columns(পূর্ণসংখ্যা flags, অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	পূর্ণসংখ্যা error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_COLUMN) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_COLUMN) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_আকার_COLUMN) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_COLUMN) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_TIME_COLUMN) ?
					"span.time { display: none; }\n" : "");
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
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

bool dirlist_generate_title(ধ্রুবক অক্ষর *title, অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	ধ্রুবক অক্ষর *stylesheet;
	nserror err;
	পূর্ণসংখ্যা error;

	যদি (title == ফাঁকা)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	যদি (err != NSERROR_OK) {
		ফেরত false;
	}

	error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
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
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
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

bool dirlist_generate_parent_link(ধ্রুবক অক্ষর *parent, অক্ষর *বাফার,
		পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	পূর্ণসংখ্যা error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
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

bool dirlist_generate_headings(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	পূর্ণসংখ্যা error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
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
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
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

bool dirlist_generate_row(bool even, bool directory, nsurl *url, অক্ষর *name,
		ধ্রুবক অক্ষর *mimetype, long long আকার_2, অক্ষর *date, অক্ষর *time,
		অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	ধ্রুবক অক্ষর *unit;
	অক্ষর আকার_string[100];
	পূর্ণসংখ্যা error;

	যদি (আকার_2 < 0) {
		unit = "";
		strncpy(আকার_string, "", আকার আকার_string);
	} অন্যথা {
		unit = messages_get(dirlist_filesize_unit((unsigned long)আকার_2));
		snprintf(আকার_string, আকার আকার_string, "%d",
				dirlist_filesize_মান((unsigned long)আকার_2));
	}

	error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
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
			name, mimetype, আকার_string, unit, date, time);
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
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

bool dirlist_generate_bottom(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য)
{
	পূর্ণসংখ্যা error = snprintf(বাফার, বাফার_দৈর্ঘ্য,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	যদি (error < 0 || error >= বাফার_দৈর্ঘ্য)
		/* Error or buffer too small */
		ফেরত false;
	অন্যথা
		/* OK */
		ফেরত true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

পূর্ণসংখ্যা dirlist_filesize_calculate(unsigned long *bytesize)
{
	পূর্ণসংখ্যা i = 0;
	যখন (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		যদি (i == 3)
			বিরতি;
	}
	ফেরত i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

পূর্ণসংখ্যা dirlist_filesize_মান(unsigned long bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ফেরত (পূর্ণসংখ্যা)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

অক্ষর* dirlist_filesize_unit(unsigned long bytesize)
{
	ধ্রুবক অক্ষর* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ফেরত (অক্ষর*)units[dirlist_filesize_calculate(&bytesize)];
}
