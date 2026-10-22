/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
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

/**
 * \file
 * Interface to HTML imagemap
 */

#ifndef NETSURF_HTML_IMAGEMAP_H
#define NETSURF_HTML_IMAGEMAP_H

#অন্তর্ভাৱ <dom/dom.h>

সংরচনা html_content;
সংরচনা hlcache_handle;
সংরচনা nsurl;

শূন্য imagemap_destroy(সংরচনা html_content *c);
শূন্য imagemap_dump(সংরচনা html_content *c);
nserror imagemap_extract(সংরচনা html_content *c);

সংরচনা nsurl *imagemap_get(সংরচনা html_content *c, ধ্রুৱ ৱর্ণ *চাবি,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ধ্রুৱ ৱর্ণ **target);

#endif
