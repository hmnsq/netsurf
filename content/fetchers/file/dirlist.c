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

#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>

#ضمن "utils/nsurl.h"
#ضمن "utils/messages.h"
#ضمن "utils/nscolour.h"

#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/types.h"
#ضمن "netsurf/plot_style.h"

#ضمن "dirlist.h"
#ضمن "desktop/system_colour.h"

ساكن صحيح dirlist_filesize_calculate(بنانشانی لما *bytesize);
ساكن صحيح dirlist_filesize_قدر(بنانشانی لما bytesize);
ساكن حرف* dirlist_filesize_unit(بنانشانی لما bytesize);


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

bool dirlist_generate_چوٹی(حرف *بفر, صحيح بفر_لمائی)
{
	صحيح error = snprintf(بفر, بفر_لمائی,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;

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

bool dirlist_generate_hide_columns(صحيح flags, حرف *بفر, صحيح بفر_لمائی)
{
	صحيح error = snprintf(بفر, بفر_لمائی,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_کالم) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_کالم) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_ناپ_کالم) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_کالم) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_ویلا_کالم) ?
					"span.time { display: none; }\n" : "");
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
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

bool dirlist_generate_title(ثابت حرف *title, حرف *بفر, صحيح بفر_لمائی)
{
	ثابت حرف *stylesheet;
	nserror err;
	صحيح error;

	إذا (title == عدم)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	إذا (err != NSERROR_OK) {
		أرجع false;
	}

	error = snprintf(بفر, بفر_لمائی,
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
			رنگ_rb_وٹاندرا(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
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

bool dirlist_generate_ماپا_link(ثابت حرف *ماپا, حرف *بفر,
		صحيح بفر_لمائی)
{
	صحيح error = snprintf(بفر, بفر_لمائی,
			"<p><a href=\"%s\">%s</a></p>",
			ماپا, messages_get("FileParent"));
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
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

bool dirlist_generate_headings(حرف *بفر, صحيح بفر_لمائی)
{
	صحيح error = snprintf(بفر, بفر_لمائی,
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
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
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
		ثابت حرف *mimetype, لما لما ناپ, حرف *date, حرف *time,
		حرف *بفر, صحيح بفر_لمائی)
{
	ثابت حرف *unit;
	حرف ناپ_string[100];
	صحيح error;

	إذا (ناپ < 0) {
		unit = "";
		strncpy(ناپ_string, "", حجم ناپ_string);
	} وإلا {
		unit = messages_get(dirlist_filesize_unit((بنانشانی لما)ناپ));
		snprintf(ناپ_string, حجم ناپ_string, "%d",
				dirlist_filesize_قدر((بنانشانی لما)ناپ));
	}

	error = snprintf(بفر, بفر_لمائی,
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
			name, mimetype, ناپ_string, unit, date, time);
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
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

bool dirlist_generate_bottom(حرف *بفر, صحيح بفر_لمائی)
{
	صحيح error = snprintf(بفر, بفر_لمائی,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	إذا (error < 0 || error >= بفر_لمائی)
		/* Error or buffer too small */
		أرجع false;
	وإلا
		/* OK */
		أرجع true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

صحيح dirlist_filesize_calculate(بنانشانی لما *bytesize)
{
	صحيح i = 0;
	طالما (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		إذا (i == 3)
			اكسر;
	}
	أرجع i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

صحيح dirlist_filesize_قدر(بنانشانی لما bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	أرجع (صحيح)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

حرف* dirlist_filesize_unit(بنانشانی لما bytesize)
{
	ثابت حرف* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	أرجع (حرف*)units[dirlist_filesize_calculate(&bytesize)];
}
