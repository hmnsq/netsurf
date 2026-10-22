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

#include <dom/dom.h>

sa_nracanaa html_content;
sa_nracanaa hlcache_handle;
sa_nracanaa nsurl;

void imagemap_destroy(sa_nracanaa html_content *c);
void imagemap_dump(sa_nracanaa html_content *c);
nserror imagemap_extract(sa_nracanaa html_content *c);

sa_nracanaa nsurl *imagemap_get(sa_nracanaa html_content *c, const var_na *key,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		const var_na **target);

#endif
