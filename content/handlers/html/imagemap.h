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

#અન્તર્ભાવ <dom/dom.h>

સંરચના html_content;
સંરચના hlcache_handle;
સંરચના nsurl;

શૂન્ય imagemap_destroy(સંરચના html_content *c);
શૂન્ય imagemap_dump(સંરચના html_content *c);
nserror imagemap_extract(સંરચના html_content *c);

સંરચના nsurl *imagemap_get(સંરચના html_content *c, અચલ અક્ષર *ચાવી,
		unsigned long x, unsigned long y,
		unsigned long click_x, unsigned long click_y,
		અચલ અક્ષર **target);

#endif
