/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
 * Copyright 2015 Daniel Dilverstone <dsilvers@netsurf-browser.org>
 * Copyright 2016 Michael Drake <tlsa@netsurf-browser.org>
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
 * Duktapeish implementation of javascript engine functions, prototypes.
 */

#ifndef DUKKY_H
#define DUKKY_H

duk_ret_t dukky_create_object(duk_context *ctx, ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ args);
duk_bool_t dukky_ತಳ್ಳು_ಗಂಟು_stacked(duk_context *ctx);
duk_bool_t dukky_ತಳ್ಳು_ಗಂಟು(duk_context *ctx, ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು);
ಶೂನ್ಯ dukky_inject_not_ctr(duk_context *ctx, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *name);
ಶೂನ್ಯ dukky_register_event_listener_for(duk_context *ctx,
				       ಸಂರಚನಾ dom_element *ele,
				       dom_string *name,
				       bool capture);
bool dukky_get_current_ಮೌಲ್ಯ_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et);
ಶೂನ್ಯ dukky_ತಳ್ಳು_event(duk_context *ctx, dom_event *evt);
bool dukky_event_target_ತಳ್ಳು_listeners(duk_context *ctx, bool dont_create);

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	ELF_CAPTURE = 1 << 0,
	ELF_PASSIVE = 1 << 1,
	ELF_ONCE    = 1 << 2,
	ELF_NONE    = 0
} event_listener_flags;

ಶೂನ್ಯ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx);

/* pcall something, and if it errored, also dump the error to the log */
duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout);

/* Push a generics function onto the stack */
ಶೂನ್ಯ dukky_ತಳ್ಳು_generics(duk_context *ctx, ಧ್ರುವ ವರ್ಣ *generic);

/* Log the current stack frame if possible */
ಶೂನ್ಯ dukky_log_ರಾಶಿ_frame(duk_context *ctx, ಧ್ರುವ ವರ್ಣ * reason);

#endif
