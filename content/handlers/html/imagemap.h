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

#నిర్వచితం_కాకపోతే NETSURF_HTML_IMAGEMAP_H
#నిర్వచించు NETSURF_HTML_IMAGEMAP_H

#అన్తర్భావ <dom/dom.h>

నిర్మాణం html_content;
నిర్మాణం hlcache_handle;
నిర్మాణం nsurl;

శూన్య imagemap_destroy(నిర్మాణం html_content *c);
శూన్య imagemap_dump(నిర్మాణం html_content *c);
nserror imagemap_వెలికితీయి(నిర్మాణం html_content *c);

నిర్మాణం nsurl *imagemap_get(నిర్మాణం html_content *c, స్థిరరాశి అక్షరం *కీలకం,
		గుర్తులేని దీర్ఘ x, గుర్తులేని దీర్ఘ y,
		గుర్తులేని దీర్ఘ click_x, గుర్తులేని దీర్ఘ click_y,
		స్థిరరాశి అక్షరం **లక్ష్యం);

#అయితే_ముగింపు
