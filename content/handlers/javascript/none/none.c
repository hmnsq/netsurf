/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
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
 * Dummy implementation of javascript engine functions.
 */

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"

#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"

ಶೂನ್ಯ js_initialise(ಶೂನ್ಯ)
{
	nsoption_ಗಣ_bool(enable_javascript, false);
}

ಶೂನ್ಯ js_finalise(ಶೂನ್ಯ)
{
}

nserror js_newheap(ಪೂರ್ಣಾಂಕ timeout, jsheap **ರಾಶಿ)
{
	*ರಾಶಿ = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಶೂನ್ಯ js_destroyheap(jsheap *ರಾಶಿ)
{
}

nserror js_newthread(jsheap *ರಾಶಿ, ಶೂನ್ಯ *win_priv, ಶೂನ್ಯ *doc_priv, jsthread **thread)
{
	*thread = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಶೂನ್ಯ js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ಸ್ಥಿರಾಂಕ uint8_t *txt, size_t txtlen, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಹಿಂತಿರುಗಿ true;
}

bool js_fire_event(jsthread *thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ರಚನೆ dom_document *doc, ರಚನೆ dom_ಗಂಟು *ಗುರಿ)
{
	ಹಿಂತಿರುಗಿ true;
}

ಶೂನ್ಯ js_handle_new_element(jsthread *thread, ರಚನೆ dom_element *ಗಂಟು)
{
}

ಶೂನ್ಯ js_event_cleanup(jsthread *thread, ರಚನೆ dom_event *evt)
{
}
