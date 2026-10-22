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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dirlist_filesize_calculate(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *bytesize);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dirlist_filesize_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bytesize);
ಸ್ಥಿರ ಅಕ್ಷರ* dirlist_filesize_unit(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bytesize);


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

bool dirlist_generate_ತುದಿ(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;

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

bool dirlist_generate_hide_columns(ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_ಸ್ತಂಭ) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_ಸ್ತಂಭ) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_ಗಾತ್ರ_ಸ್ತಂಭ) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_ಸ್ತಂಭ) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_ಸಮಯ_ಸ್ತಂಭ) ?
					"span.time { display: none; }\n" : "");
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
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

bool dirlist_generate_title(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title, ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *stylesheet;
	nserror err;
	ಪೂರ್ಣಾಂಕ error;

	ಆದರೆ (title == ಖಾಲಿ)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ಆದರೆ (err != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
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
			ಬಣ್ಣ_rb_ಅದಲುಬದಲು(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
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

bool dirlist_generate_ಪೋಷಕ_link(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೋಷಕ, ಅಕ್ಷರ *ಬಫರ್,
		ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
			"<p><a href=\"%s\">%s</a></p>",
			ಪೋಷಕ, messages_get("FileParent"));
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
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

bool dirlist_generate_headings(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
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
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
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

bool dirlist_generate_ಅಡ್ಡಸಾಲು(bool even, bool directory, nsurl *url, ಅಕ್ಷರ *name,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mimetype, ದೀರ್ಘ ದೀರ್ಘ ಗಾತ್ರ_2, ಅಕ್ಷರ *date, ಅಕ್ಷರ *time,
		ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *unit;
	ಅಕ್ಷರ ಗಾತ್ರ_string[100];
	ಪೂರ್ಣಾಂಕ error;

	ಆದರೆ (ಗಾತ್ರ_2 < 0) {
		unit = "";
		strncpy(ಗಾತ್ರ_string, "", ಗಾತ್ರ ಗಾತ್ರ_string);
	} ಇಲ್ಲದಿದ್ದರೆ {
		unit = messages_get(dirlist_filesize_unit((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಗಾತ್ರ_2));
		snprintf(ಗಾತ್ರ_string, ಗಾತ್ರ ಗಾತ್ರ_string, "%d",
				dirlist_filesize_ಮೌಲ್ಯ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಗಾತ್ರ_2));
	}

	error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
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
			name, mimetype, ಗಾತ್ರ_string, unit, date, time);
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
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

bool dirlist_generate_bottom(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ error = snprintf(ಬಫರ್, ಬಫರ್_ಉದ್ದಳತೆ,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ಆದರೆ (error < 0 || error >= ಬಫರ್_ಉದ್ದಳತೆ)
		/* Error or buffer too small */
		ಹಿಂತಿರುಗಿ false;
	ಇಲ್ಲದಿದ್ದರೆ
		/* OK */
		ಹಿಂತಿರುಗಿ true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

ಪೂರ್ಣಾಂಕ dirlist_filesize_calculate(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *bytesize)
{
	ಪೂರ್ಣಾಂಕ i = 0;
	ಆಗ (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ಆದರೆ (i == 3)
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

ಪೂರ್ಣಾಂಕ dirlist_filesize_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

ಅಕ್ಷರ* dirlist_filesize_unit(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bytesize)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)units[dirlist_filesize_calculate(&bytesize)];
}
