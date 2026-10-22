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

#اگر_غیر_تعریف_شدہ NETSURF_HTML_IMAGEMAP_H
#تعریف_کرو NETSURF_HTML_IMAGEMAP_H

#شامل <dom/dom.h>

ساخت html_content;
ساخت hlcache_handle;
ساخت nsurl;

خالی imagemap_destroy(ساخت html_content *c);
خالی imagemap_dump(ساخت html_content *c);
nserror imagemap_نکالو(ساخت html_content *c);

ساخت nsurl *imagemap_get(ساخت html_content *c, ثابت حرف *کلید,
		غیرعلامتی طویل x, غیرعلامتی طویل y,
		غیرعلامتی طویل click_x, غیرعلامتی طویل click_y,
		ثابت حرف **ہدف);

#اگر_ختم
