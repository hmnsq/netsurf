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

#که_نه_تعریف_شوی DUKKY_H
#تعریف_کړه DUKKY_H

duk_ret_t dukky_create_object(duk_context *ctx, ثابت حرف *name, صحيح دلیلونه);
duk_bool_t dukky_ټېل_غوټه_stacked(duk_context *ctx);
duk_bool_t dukky_ټېل_غوټه(duk_context *ctx, بنية dom_غوټه *غوټه);
فراغ dukky_inject_not_ctr(duk_context *ctx, صحيح idx, ثابت حرف *name);
فراغ dukky_ثبتوونکی_event_listener_for(duk_context *ctx,
				       بنية dom_element *ele,
				       dom_string *name,
				       bool capture);
bool dukky_get_current_ارزښت_د_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_موخه *et);
فراغ dukky_ټېل_event(duk_context *ctx, dom_event *evt);
bool dukky_event_موخه_ټېل_listeners(duk_context *ctx, bool dont_create);

تعريفنوع تعداد {
	ELF_CAPTURE = 1 << 0,
	ELF_PASSIVE = 1 << 1,
	ELF_ONCE    = 1 << 2,
	ELF_NONE    = 0
} event_listener_flags;

فراغ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx);

/* pcall something, and if it errored, also dump the error to the log */
duk_int_t dukky_pcall(duk_context *ctx, duk_اندازه_t argc, bool reset_timeout);

/* Push a generics function onto the stack */
فراغ dukky_ټېل_generics(duk_context *ctx, ثابت حرف *generic);

/* Log the current stack frame if possible */
فراغ dukky_log_ډېری_چوکاټ(duk_context *ctx, ثابت حرف * reason);

#د_که_پای
