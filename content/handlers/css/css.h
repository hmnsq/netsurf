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

#నిర్వచితం_కాకపోతే netsurf_css_css_h_
#నిర్వచించు netsurf_css_css_h_

#అన్తర్భావ <stdint.h>

#అన్తర్భావ <libcss/libcss.h>

#అన్తర్భావ "utils/errors.h"

నిర్మాణం hlcache_handle;

/**
 * Imported stylesheet record
 */
నిర్మాణం nscss_import {
	నిర్మాణం hlcache_handle *c;	/**< Content containing sheet */
};

/**
 * Initialise the CSS content handler
 *
 * \return NSERROR_OK on success or error code on faliure
 */
nserror nscss_init(శూన్య);

/**
 * Retrieve the stylesheet object associated with a CSS content
 *
 * \param h Stylesheet content
 * \return Pointer to stylesheet object
 */
css_stylesheet *nscss_get_stylesheet(నిర్మాణం hlcache_handle *h);

/**
 * Retrieve imported stylesheets
 *
 * \param h  Stylesheet containing imports
 * \param n  Pointer to location to receive number of imports
 * \return Pointer to array of imported stylesheets
 */
నిర్మాణం nscss_import *nscss_get_imports(నిర్మాణం hlcache_handle *h, uint32_t *n);

#అయితే_ముగింపు
