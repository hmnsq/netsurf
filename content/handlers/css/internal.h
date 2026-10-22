/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#جيڪڏهن_وصف_نه_ٿيل NETSURF_CSS_اندروني_H
#وصف_ڪريو NETSURF_CSS_اندروني_H

/**
 * URL resolution callback for libcss
 *
 * \param pw    Resolution context
 * \param base  Base URI
 * \param rel   Relative URL
 * \param abs   Pointer to location to receive resolved URL
 * \return CSS_OK       on success,
 *         CSS_NOMEM    on memory exhaustion,
 *         CSS_INVALID  if resolution failed.
 */
css_error nscss_resolve_url(فراغ *pw, ثابت حرف *بنياد, lwc_string *rel, lwc_string **مطلق_قدر);

#جيڪڏهن_پورو
