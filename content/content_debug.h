/*
 * Copyright 2016 Vincent Sanders <vince@netsurf-browser.org>
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
 * Interface to content handling debug.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _NETSURF_CONTENT_CONTENT_DEBUG_H_
#ವ್ಯಾಖ್ಯಾನಿಸು _NETSURF_CONTENT_CONTENT_DEBUG_H_

#ಅನ್ತರ್ಭಾವ <stdio.h>

/**
 * Dump debug information to file.
 *
 * \param h content handle to debug.
 * \param f File to write output to.
 * \param op Debug operation type.
 */
nserror content_debug_dump(ರಚನೆ hlcache_handle *h, FILE *f, ಎಣಿಕೆ content_debug op);

#ಆದರೆ_ಅಂತ್ಯ
