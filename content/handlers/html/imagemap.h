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

#अन्तर्भाव <dom/dom.h>

संरचना html_content;
संरचना hlcache_handle;
संरचना nsurl;

शून्य imagemap_destroy(संरचना html_content *c);
शून्य imagemap_dump(संरचना html_content *c);
nserror imagemap_extract(संरचना html_content *c);

संरचना nsurl *imagemap_get(संरचना html_content *c, ध्रुव वर्ण *कुञ्चिका,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		ध्रुव वर्ण **target);

#endif
