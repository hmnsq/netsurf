/*
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
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
 * interface to generate HTML content for displaying directory listings.
 *
 * These functions should in general be called via the content interface.
 */

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_DIRLIST_H_
#నిర్వచించు NETSURF_CONTENT_DIRLIST_H_

#అన్తర్భావ <stdbool.h>

#నిర్వచించు DIRLIST_NO_NAME_స్తంభం 1
#నిర్వచించు DIRLIST_NO_TYPE_స్తంభం 1 << 1
#నిర్వచించు DIRLIST_NO_కొలత_స్తంభం 1 << 2
#నిర్వచించు DIRLIST_NO_DATE_స్తంభం 1 << 3
#నిర్వచించు DIRLIST_NO_సమయం_స్తంభం 1 << 4

నిర్మాణం nsurl;

bool dirlist_generate_శిఖరం(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_hide_columns(పూర్ణాంకం flags, అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_title(స్థిరరాశి అక్షరం *title, అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_మాతృ_link(స్థిరరాశి అక్షరం *మాతృ, అక్షరం *బఫరు,
		పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_headings(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_అడ్డువరుస(bool even, bool directory, నిర్మాణం nsurl *url,
		అక్షరం *name, స్థిరరాశి అక్షరం *mimetype, దీర్ఘ దీర్ఘ కొలత, అక్షరం *date,
		అక్షరం *time, అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);
bool dirlist_generate_bottom(అక్షరం *బఫరు, పూర్ణాంకం బఫరు_నిడివి);

#అయితే_ముగింపు
