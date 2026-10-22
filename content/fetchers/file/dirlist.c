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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nscolour.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plot_style.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "dirlist.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/system_colour.h"

ਸਥਿਰ ਪੂਰਨ dirlist_filesize_calculate(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *bytesize);
ਸਥਿਰ ਪੂਰਨ dirlist_filesize_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bytesize);
ਸਥਿਰ ਅੱਖਰ* dirlist_filesize_unit(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bytesize);


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

bool dirlist_generate_ਸਿਖਰ(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਪੂਰਨ error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;

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

bool dirlist_generate_hide_columns(ਪੂਰਨ flags, ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਪੂਰਨ error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_ਕਾਲਮ) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_ਕਾਲਮ) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_ਮਾਪ_ਕਾਲਮ) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_ਕਾਲਮ) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_ਸਮਾਂ_ਕਾਲਮ) ?
					"span.time { display: none; }\n" : "");
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
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

bool dirlist_generate_title(ਅਚਲ ਅੱਖਰ *title, ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਅਚਲ ਅੱਖਰ *stylesheet;
	nserror err;
	ਪੂਰਨ error;

	ਜੇ (title == ਨਿਰਾ)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ਜੇ (err != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
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
			ਰੰਗ_rb_ਅਦਲਾ_ਬਦਲੀ(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
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

bool dirlist_generate_ਮਾਪਾ_link(ਅਚਲ ਅੱਖਰ *ਮਾਪਾ, ਅੱਖਰ *ਬਫਰ,
		ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਪੂਰਨ error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
			"<p><a href=\"%s\">%s</a></p>",
			ਮਾਪਾ, messages_get("FileParent"));
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
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

bool dirlist_generate_headings(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਪੂਰਨ error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
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
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
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

bool dirlist_generate_ਪੰਕਤੀ(bool even, bool directory, nsurl *url, ਅੱਖਰ *name,
		ਅਚਲ ਅੱਖਰ *mimetype, ਲੰਮਾ ਲੰਮਾ ਮਾਪ, ਅੱਖਰ *date, ਅੱਖਰ *time,
		ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਅਚਲ ਅੱਖਰ *unit;
	ਅੱਖਰ ਮਾਪ_string[100];
	ਪੂਰਨ error;

	ਜੇ (ਮਾਪ < 0) {
		unit = "";
		strncpy(ਮਾਪ_string, "", ਆਕਾਰ ਮਾਪ_string);
	} ਨਹੀਂਤਾਂ {
		unit = messages_get(dirlist_filesize_unit((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਮਾਪ));
		snprintf(ਮਾਪ_string, ਆਕਾਰ ਮਾਪ_string, "%d",
				dirlist_filesize_ਮੁੱਲ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਮਾਪ));
	}

	error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
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
			name, mimetype, ਮਾਪ_string, unit, date, time);
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
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

bool dirlist_generate_bottom(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ)
{
	ਪੂਰਨ error = snprintf(ਬਫਰ, ਬਫਰ_ਲੰਮਾਈ,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ਜੇ (error < 0 || error >= ਬਫਰ_ਲੰਮਾਈ)
		/* Error or buffer too small */
		ਵਾਪਸ false;
	ਨਹੀਂਤਾਂ
		/* OK */
		ਵਾਪਸ true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

ਪੂਰਨ dirlist_filesize_calculate(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *bytesize)
{
	ਪੂਰਨ i = 0;
	ਜਦੋਂ (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ਜੇ (i == 3)
			ਰੋਕੋ;
	}
	ਵਾਪਸ i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

ਪੂਰਨ dirlist_filesize_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ਵਾਪਸ (ਪੂਰਨ)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

ਅੱਖਰ* dirlist_filesize_unit(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bytesize)
{
	ਅਚਲ ਅੱਖਰ* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ਵਾਪਸ (ਅੱਖਰ*)units[dirlist_filesize_calculate(&bytesize)];
}
