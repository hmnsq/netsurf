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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/nscolour.h"

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plot_style.h"

#ଅନ୍ତର୍ଭାଵ "dirlist.h"
#ଅନ୍ତର୍ଭାଵ "desktop/system_colour.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirlist_filesize_calculate(unsigned long *bytesize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirlist_filesize_ମୂଲ୍ୟ(unsigned long bytesize);
ସ୍ଥିର ଅକ୍ଷର* dirlist_filesize_unit(unsigned long bytesize);


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

bool dirlist_generate_ଶୀର୍ଷ(ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = snprintf(buffer, buffer_length,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;

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

bool dirlist_generate_hide_columns(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = snprintf(buffer, buffer_length,
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
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
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

bool dirlist_generate_title(ଅଚଳ ଅକ୍ଷର *title, ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ଅଚଳ ଅକ୍ଷର *stylesheet;
	nserror err;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;

	ଯଦି (title == ଖାଲି)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ଯଦି (err != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
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
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
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

bool dirlist_generate_parent_link(ଅଚଳ ଅକ୍ଷର *parent, ଅକ୍ଷର *buffer,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = snprintf(buffer, buffer_length,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
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

bool dirlist_generate_headings(ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = snprintf(buffer, buffer_length,
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
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
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

bool dirlist_generate_row(bool even, bool directory, nsurl *url, ଅକ୍ଷର *name,
		ଅଚଳ ଅକ୍ଷର *mimetype, long long size, ଅକ୍ଷର *date, ଅକ୍ଷର *time,
		ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ଅଚଳ ଅକ୍ଷର *unit;
	ଅକ୍ଷର size_string[100];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;

	ଯଦି (size < 0) {
		unit = "";
		strncpy(size_string, "", ଆକାର size_string);
	} ଅନ୍ୟଥା {
		unit = messages_get(dirlist_filesize_unit((unsigned long)size));
		snprintf(size_string, ଆକାର size_string, "%d",
				dirlist_filesize_ମୂଲ୍ୟ((unsigned long)size));
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
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
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

bool dirlist_generate_bottom(ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buffer_length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = snprintf(buffer, buffer_length,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ଯଦି (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ଫେରନ୍ତୁ false;
	ଅନ୍ୟଥା
		/* OK */
		ଫେରନ୍ତୁ true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirlist_filesize_calculate(unsigned long *bytesize)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	ଯେତେବେଳେ (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ଯଦି (i == 3)
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirlist_filesize_ମୂଲ୍ୟ(unsigned long bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

ଅକ୍ଷର* dirlist_filesize_unit(unsigned long bytesize)
{
	ଅଚଳ ଅକ୍ଷର* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ଫେରନ୍ତୁ (ଅକ୍ଷର*)units[dirlist_filesize_calculate(&bytesize)];
}
