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

#अगर_अघोषित NETSURF_HTML_IMAGEMAP_H
#घोषणा NETSURF_HTML_IMAGEMAP_H

#समावेश <dom/dom.h>

काष्ठा html_content;
काष्ठा hlcache_handle;
काष्ठा nsurl;

व्योम imagemap_destroy(काष्ठा html_content *c);
व्योम imagemap_dump(काष्ठा html_content *c);
nserror imagemap_extract(काष्ठा html_content *c);

काष्ठा nsurl *imagemap_get(काष्ठा html_content *c, स्थिर अक्षर *कुंजी,
		अचिन्हित दीर्घ x, अचिन्हित दीर्घ y,
		अचिन्हित दीर्घ click_x, अचिन्हित दीर्घ click_y,
		स्थिर अक्षर **target);

#पूर्ण_अगर
