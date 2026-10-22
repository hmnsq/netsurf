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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DUKKY_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DUKKY_H

duk_ret_t dukky_create_object(duk_context *ctx, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ ਦਲੀਲਾਂ);
duk_bool_t dukky_ਧੱਕੋ_ਗੰਢ_stacked(duk_context *ctx);
duk_bool_t dukky_ਧੱਕੋ_ਗੰਢ(duk_context *ctx, ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ);
ਖਾਲੀ dukky_inject_not_ctr(duk_context *ctx, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *name);
ਖਾਲੀ dukky_ਰਜਿਸਟਰ_event_listener_for(duk_context *ctx,
				       ਬਣਤਰ dom_element *ele,
				       dom_string *name,
				       bool capture);
bool dukky_get_current_ਮੁੱਲ_ਦਾ_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_ਨਿਸ਼ਾਨਾ *et);
ਖਾਲੀ dukky_ਧੱਕੋ_event(duk_context *ctx, dom_event *evt);
bool dukky_event_ਨਿਸ਼ਾਨਾ_ਧੱਕੋ_listeners(duk_context *ctx, bool dont_create);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	ELF_CAPTURE = 1 << 0,
	ELF_PASSIVE = 1 << 1,
	ELF_ONCE    = 1 << 2,
	ELF_NONE    = 0
} event_listener_flags;

ਖਾਲੀ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx);

/* pcall something, and if it errored, also dump the error to the log */
duk_int_t dukky_pcall(duk_context *ctx, duk_ਮਾਪ_t argc, bool reset_timeout);

/* Push a generics function onto the stack */
ਖਾਲੀ dukky_ਧੱਕੋ_generics(duk_context *ctx, ਅਚਲ ਅੱਖਰ *generic);

/* Log the current stack frame if possible */
ਖਾਲੀ dukky_log_ਢੇਰ_ਚੌਖਟਾ(duk_context *ctx, ਅਚਲ ਅੱਖਰ * reason);

#ਜੇ_ਅੰਤ
