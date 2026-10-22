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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/nscolour.h"

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "netsurf/plot_style.h"

#అన్తర్భావ "dirlist.h"
#అన్తర్భావ "desktop/system_colour.h"

స్థిర పూర్ణాంకం dirlist_filesize_calculate(గుర్తులేని దీర్ఘ *bytesize);
స్థిర పూర్ణాంకం dirlist_filesize_విలువ(గుర్తులేని దీర్ఘ bytesize);
స్థిర అక్షరం* dirlist_filesize_unit(గుర్తులేని దీర్ఘ bytesize);


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

bool dirlist_generate_శిఖరం(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	పూర్ణాంకం error = snprintf(బఫరు, బఫరు_నిడివి,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;

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

bool dirlist_generate_hide_columns(పూర్ణాంకం flags, అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	పూర్ణాంకం error = snprintf(బఫరు, బఫరు_నిడివి,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_స్తంభం) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_స్తంభం) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_కొలత_స్తంభం) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_స్తంభం) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_సమయం_స్తంభం) ?
					"span.time { display: none; }\n" : "");
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
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

bool dirlist_generate_title(స్థిరరాశి అక్షరం *title, అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	స్థిరరాశి అక్షరం *stylesheet;
	nserror err;
	పూర్ణాంకం error;

	ఒకవేళ (title == ఖాళీ)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ఒకవేళ (err != NSERROR_OK) {
		తిరిగి false;
	}

	error = snprintf(బఫరు, బఫరు_నిడివి,
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
			రంగు_rb_మార్పిడి(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
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

bool dirlist_generate_మాతృ_link(స్థిరరాశి అక్షరం *మాతృ, అక్షరం *బఫరు,
		పూర్ణాంకం బఫరు_నిడివి)
{
	పూర్ణాంకం error = snprintf(బఫరు, బఫరు_నిడివి,
			"<p><a href=\"%s\">%s</a></p>",
			మాతృ, messages_get("FileParent"));
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
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

bool dirlist_generate_headings(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	పూర్ణాంకం error = snprintf(బఫరు, బఫరు_నిడివి,
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
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
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

bool dirlist_generate_అడ్డువరుస(bool even, bool directory, nsurl *url, అక్షరం *name,
		స్థిరరాశి అక్షరం *mimetype, దీర్ఘ దీర్ఘ కొలత, అక్షరం *date, అక్షరం *time,
		అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	స్థిరరాశి అక్షరం *unit;
	అక్షరం కొలత_string[100];
	పూర్ణాంకం error;

	ఒకవేళ (కొలత < 0) {
		unit = "";
		strncpy(కొలత_string, "", పరిమాణం కొలత_string);
	} లేకపోతే {
		unit = messages_get(dirlist_filesize_unit((గుర్తులేని దీర్ఘ)కొలత));
		snprintf(కొలత_string, పరిమాణం కొలత_string, "%d",
				dirlist_filesize_విలువ((గుర్తులేని దీర్ఘ)కొలత));
	}

	error = snprintf(బఫరు, బఫరు_నిడివి,
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
			name, mimetype, కొలత_string, unit, date, time);
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
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

bool dirlist_generate_bottom(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి)
{
	పూర్ణాంకం error = snprintf(బఫరు, బఫరు_నిడివి,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ఒకవేళ (error < 0 || error >= బఫరు_నిడివి)
		/* Error or buffer too small */
		తిరిగి false;
	లేకపోతే
		/* OK */
		తిరిగి true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

పూర్ణాంకం dirlist_filesize_calculate(గుర్తులేని దీర్ఘ *bytesize)
{
	పూర్ణాంకం i = 0;
	అయితే (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ఒకవేళ (i == 3)
			ఆపు;
	}
	తిరిగి i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

పూర్ణాంకం dirlist_filesize_విలువ(గుర్తులేని దీర్ఘ bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	తిరిగి (పూర్ణాంకం)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

అక్షరం* dirlist_filesize_unit(గుర్తులేని దీర్ఘ bytesize)
{
	స్థిరరాశి అక్షరం* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	తిరిగి (అక్షరం*)units[dirlist_filesize_calculate(&bytesize)];
}
