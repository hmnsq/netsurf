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

#شامل "utils/config.h"

#اگر_غیر_تعریف_شدہ HAVE_STRPTIME
#تعریف_منسوخ DUK_USE_DATE_PRS_STRPTIME
#تعریف_منسوخ DUK_USE_DATE_PRS_GETDATE
#تعریف_منسوخ DUK_USE_DATE_PARSE_STRING
#اگر_ختم

#تعریف_کرو DUK_USE_FASTINT
#تعریف_کرو DUK_USE_REGEXP_CANON_WORKAROUND

/* Required for execution timeout checking */
#تعریف_کرو DUK_USE_مداخلت_شمار

خارجی duk_bool_t dukky_check_timeout(خالی *udata);
#تعریف_کرو DUK_USE_EXEC_TIMEOUT_CHECK dukky_check_timeout

#شامل "netsurf/inttypes.h"
