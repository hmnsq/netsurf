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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_IMAGEMAP_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_IMAGEMAP_H

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

ਬਣਤਰ html_content;
ਬਣਤਰ hlcache_handle;
ਬਣਤਰ nsurl;

ਖਾਲੀ imagemap_destroy(ਬਣਤਰ html_content *c);
ਖਾਲੀ imagemap_dump(ਬਣਤਰ html_content *c);
nserror imagemap_ਕੱਢੋ(ਬਣਤਰ html_content *c);

ਬਣਤਰ nsurl *imagemap_get(ਬਣਤਰ html_content *c, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ x, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ y,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_x, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ click_y,
		ਅਚਲ ਅੱਖਰ **ਨਿਸ਼ਾਨਾ);

#ਜੇ_ਅੰਤ
