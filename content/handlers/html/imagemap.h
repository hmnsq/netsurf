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

#ضمن <dom/dom.h>

بنية html_content;
بنية hlcache_handle;
بنية nsurl;

فراغ imagemap_destroy(بنية html_content *c);
فراغ imagemap_dump(بنية html_content *c);
nserror imagemap_extract(بنية html_content *c);

بنية nsurl *imagemap_get(بنية html_content *c, ثابت حرف *مبودي,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ثابت حرف **target);

#endif
