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

#ضمن "utils/config.h"

#اگر_تعریف_نشده HAVE_STRPTIME
#لغو_تعریف DUK_USE_DATE_PRS_STRPTIME
#لغو_تعریف DUK_USE_DATE_PRS_GETDATE
#لغو_تعریف DUK_USE_DATE_PARSE_STRING
#پایان_اگر

#تعریف_کن DUK_USE_FASTINT
#تعریف_کن DUK_USE_REGEXP_CANON_WORKAROUND

/* Required for execution timeout checking */
#تعریف_کن DUK_USE_وقفه_COUNTER

خارجي duk_bool_t dukky_check_timeout(فراغ *udata);
#تعریف_کن DUK_USE_EXEC_TIMEOUT_CHECK dukky_check_timeout

#ضمن "netsurf/inttypes.h"
