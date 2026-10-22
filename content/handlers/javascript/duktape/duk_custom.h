/*
 * Copyright 2015 Daniel Silverstone <dsilvers@netsurf-browser.org>
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

/** \file
 * Custom configuration for duktape
 */

#అన్తర్భావ "utils/config.h"

#నిర్వచితం_కాకపోతే HAVE_STRPTIME
#నిర్వచనం_రద్దు DUK_USE_DATE_PRS_STRPTIME
#నిర్వచనం_రద్దు DUK_USE_DATE_PRS_GETDATE
#నిర్వచనం_రద్దు DUK_USE_DATE_PARSE_STRING
#అయితే_ముగింపు

#నిర్వచించు DUK_USE_FASTINT
#నిర్వచించు DUK_USE_REGEXP_CANON_WORKAROUND

/* Required for execution timeout checking */
#నిర్వచించు DUK_USE_అంతరాయం_COUNTER

బాహ్య duk_bool_t dukky_check_timeout(శూన్య *udata);
#నిర్వచించు DUK_USE_EXEC_TIMEOUT_CHECK dukky_check_timeout

#అన్తర్భావ "netsurf/inttypes.h"
