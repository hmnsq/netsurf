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

#अन्तर्भाव "utils/config.h"

#यदि_अपरिभाषितम् HAVE_STRPTIME
#अपरिभाषय DUK_USE_DATE_PRS_STRPTIME
#अपरिभाषय DUK_USE_DATE_PRS_GETDATE
#अपरिभाषय DUK_USE_DATE_PARSE_STRING
#यदि_अन्तः

#परिभाषय DUK_USE_FASTINT
#परिभाषय DUK_USE_REGEXP_CANON_WORKAROUND

/* Required for execution timeout checking */
#परिभाषय DUK_USE_व्यवधानम्_गणक

बाह्य duk_bool_t dukky_check_timeout(शून्य *udata);
#परिभाषय DUK_USE_EXEC_TIMEOUT_CHECK dukky_check_timeout

#अन्तर्भाव "netsurf/inttypes.h"
