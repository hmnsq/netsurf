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

#অন্তর্ভাব <dom/dom.h>

কাঠামো html_content;
কাঠামো hlcache_handle;
কাঠামো nsurl;

শূন্য imagemap_destroy(কাঠামো html_content *c);
শূন্য imagemap_dump(কাঠামো html_content *c);
nserror imagemap_extract(কাঠামো html_content *c);

কাঠামো nsurl *imagemap_get(কাঠামো html_content *c, ধ্রুবক অক্ষর *চাবি,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ধ্রুবক অক্ষর **target);

#endif
