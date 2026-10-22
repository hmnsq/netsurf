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

duk_ret_t dukky_create_object(duk_context *ctx, ধ্রুব বর্ণ *name, পূর্ণাঙ্ক args);
duk_bool_t dukky_ঠেলো_গ্রন্থি_stacked(duk_context *ctx);
duk_bool_t dukky_ঠেলো_গ্রন্থি(duk_context *ctx, সংরচনা dom_গ্রন্থি *গ্রন্থি);
শূন্য dukky_inject_not_ctr(duk_context *ctx, পূর্ণাঙ্ক idx, ধ্রুব বর্ণ *name);
শূন্য dukky_register_event_listener_for(duk_context *ctx,
				       সংরচনা dom_element *ele,
				       dom_string *name,
				       bool capture);
bool dukky_get_current_মান_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et);
শূন্য dukky_ঠেলো_event(duk_context *ctx, dom_event *evt);
bool dukky_event_target_ঠেলো_listeners(duk_context *ctx, bool dont_create);

প্রকারনির্দেশ গণনা {
	ELF_CAPTURE = 1 << 0,
	ELF_PASSIVE = 1 << 1,
	ELF_ONCE    = 1 << 2,
	ELF_NONE    = 0
} event_listener_flags;

শূন্য dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx);

/* pcall something, and if it errored, also dump the error to the log */
duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout);

/* Push a generics function onto the stack */
শূন্য dukky_ঠেলো_generics(duk_context *ctx, ধ্রুব বর্ণ *generic);

/* Log the current stack frame if possible */
শূন্য dukky_log_স্তূপ_frame(duk_context *ctx, ধ্রুব বর্ণ * reason);

#endif
