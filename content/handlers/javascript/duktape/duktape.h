/*
 *  Duktape public API for Duktape 2.7.0.
 *
 *  See the API reference for documentation on call semantics.  The exposed,
 *  supported API is between the "BEGIN PUBLIC API" and "END PUBLIC API"
 *  comments.  Other parts of the header are Duktape internal and related to
 *  e.g. platform/compiler/feature detection.
 *
 *  Git commit external (external).
 *  Git branch external.
 *
 *  See Duktape AUTHORS.rst and LICENSE.txt for copyright and
 *  licensing information.
 */

/* LICENSE.txt */
/*
 *  ===============
 *  Duktape license
 *  ===============
 *  
 *  (http://opensource.org/licenses/MIT)
 *  
 *  Copyright (c) 2013-present by Duktape authors (see AUTHORS.rst)
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

/* AUTHORS.rst */
/*
 *  ===============
 *  Duktape authors
 *  ===============
 *  
 *  Copyright
 *  =========
 *  
 *  Duktape copyrights are held by its authors.  Each author has a copyright
 *  to their contribution, and agrees to irrevocably license the contribution
 *  under the Duktape ``LICENSE.txt``.
 *  
 *  Authors
 *  =======
 *  
 *  Please include an e-mail address, a link to your GitHub profile, or something
 *  similar to allow your contribution to be identified accurately.
 *  
 *  The following people have contributed code, website contents, or Wiki contents,
 *  and agreed to irrevocably license their contributions under the Duktape
 *  ``LICENSE.txt`` (in order of appearance):
 *  
 *  * Sami Vaarala <sami.vaarala@iki.fi>
 *  * Niki Dobrev
 *  * Andreas \u00d6man <andreas@lonelycoder.com>
 *  * L\u00e1szl\u00f3 Lang\u00f3 <llango.u-szeged@partner.samsung.com>
 *  * Legimet <legimet.calc@gmail.com>
 *  * Karl Skomski <karl@skomski.com>
 *  * Bruce Pascoe <fatcerberus1@gmail.com>
 *  * Ren\u00e9 Hollander <rene@rene8888.at>
 *  * Julien Hamaide (https://github.com/crazyjul)
 *  * Sebastian G\u00f6tte (https://github.com/jaseg)
 *  * Tomasz Magulski (https://github.com/magul)
 *  * \D. Bohdan (https://github.com/dbohdan)
 *  * Ond\u0159ej Jirman (https://github.com/megous)
 *  * Sa\u00fal Ibarra Corretg\u00e9 <saghul@gmail.com>
 *  * Jeremy HU <huxingyi@msn.com>
 *  * Ole Andr\u00e9 Vadla Ravn\u00e5s (https://github.com/oleavr)
 *  * Harold Brenes (https://github.com/harold-b)
 *  * Oliver Crow (https://github.com/ocrow)
 *  * Jakub Ch\u0142api\u0144ski (https://github.com/jchlapinski)
 *  * Brett Vickers (https://github.com/beevik)
 *  * Dominik Okwieka (https://github.com/okitec)
 *  * Remko Tron\u00e7on (https://el-tramo.be)
 *  * Romero Malaquias (rbsm@ic.ufal.br)
 *  * Michael Drake <michael.drake@codethink.co.uk>
 *  * Steven Don (https://github.com/shdon)
 *  * Simon Stone (https://github.com/sstone1)
 *  * \J. McC. (https://github.com/jmhmccr)
 *  * Jakub Nowakowski (https://github.com/jimvonmoon)
 *  * Tommy Nguyen (https://github.com/tn0502)
 *  * Fabrice Fontaine (https://github.com/ffontaine)
 *  * Christopher Hiller (https://github.com/boneskull)
 *  * Gonzalo Diethelm (https://github.com/gonzus)
 *  * Michal Kasperek (https://github.com/michalkas)
 *  * Andrew Janke (https://github.com/apjanke)
 *  * Steve Fan (https://github.com/stevefan1999)
 *  * Edward Betts (https://github.com/edwardbetts)
 *  * Ozhan Duz (https://github.com/webfolderio)
 *  * Akos Kiss (https://github.com/akosthekiss)
 *  * TheBrokenRail (https://github.com/TheBrokenRail)
 *  * Jesse Doyle (https://github.com/jessedoyle)
 *  * Gero Kuehn (https://github.com/dc6jgk)
 *  * James Swift (https://github.com/phraemer)
 *  * Luis de Bethencourt (https://github.com/luisbg)
 *  * Ian Whyman (https://github.com/v00d00)
 *  * Rick Sayre (https://github.com/whorfin)
 *  * Craig Leres (https://github.com/leres)
 *  * Maurici Abad (https://github.com/mauriciabad)
 *  * Nancy Li (https://github.com/NancyLi1013)
 *  * William Parks (https://github.com/WilliamParks)
 *  * Sam Hellawell (https://github.com/samhellawell)
 *  * Vladislavs Sokurenko (https://github.com/sokurenko)
 *  
 *  Other contributions
 *  ===================
 *  
 *  The following people have contributed something other than code (e.g. reported
 *  bugs, provided ideas, etc; roughly in order of appearance):
 *  
 *  * Greg Burns
 *  * Anthony Rabine
 *  * Carlos Costa
 *  * Aur\u00e9lien Bouilland
 *  * Preet Desai (Pris Matic)
 *  * judofyr (http://www.reddit.com/user/judofyr)
 *  * Jason Woofenden
 *  * Micha\u0142 Przyby\u015b
 *  * Anthony Howe
 *  * Conrad Pankoff
 *  * Jim Schimpf
 *  * Rajaran Gaunker (https://github.com/zimbabao)
 *  * Andreas \u00d6man
 *  * Doug Sanden
 *  * Josh Engebretson (https://github.com/JoshEngebretson)
 *  * Remo Eichenberger (https://github.com/remoe)
 *  * Mamod Mehyar (https://github.com/mamod)
 *  * David Demelier (https://github.com/markand)
 *  * Tim Caswell (https://github.com/creationix)
 *  * Mitchell Blank Jr (https://github.com/mitchblank)
 *  * https://github.com/yushli
 *  * Seo Sanghyeon (https://github.com/sanxiyn)
 *  * Han ChoongWoo (https://github.com/tunz)
 *  * Joshua Peek (https://github.com/josh)
 *  * Bruce E. Pascoe (https://github.com/fatcerberus)
 *  * https://github.com/Kelledin
 *  * https://github.com/sstruchtrup
 *  * Michael Drake (https://github.com/tlsa)
 *  * https://github.com/chris-y
 *  * Laurent Zubiaur (https://github.com/lzubiaur)
 *  * Neil Kolban (https://github.com/nkolban)
 *  * Wilhelm Wanecek (https://github.com/wanecek)
 *  * Andrew Janke (https://github.com/apjanke)
 *  * Unamer (https://github.com/unamer)
 *  * Karl Dahlke (eklhad@gmail.com)
 *  
 *  If you are accidentally missing from this list, send me an e-mail
 *  (``sami.vaarala@iki.fi``) and I'll fix the omission.
 */

#اگر !defined(DUKTAPE_H_INCLUDED)
#تعریف_کرو DUKTAPE_H_INCLUDED

#تعریف_کرو DUK_SINGLE_FILE

/*
 *  BEGIN PUBLIC API
 */

/*
 *  Version and Git commit identification
 */

/* Duktape version, (major * 10000) + (minor * 100) + patch.  Allows C code
 * to #if (DUK_VERSION >= NNN) against Duktape API version.  The same value
 * is also available to ECMAScript code in Duktape.version.  Unofficial
 * development snapshots have 99 for patch level (e.g. 0.10.99 would be a
 * development version after 0.10.0 but before the next official release).
 */
#تعریف_کرو DUK_نسخہ                       20700L

/* Git commit, describe, and branch for Duktape build.  Useful for
 * non-official snapshot builds so that application code can easily log
 * which Duktape snapshot was used.  Not available in the ECMAScript
 * environment.
 */
#تعریف_کرو DUK_GIT_COMMIT                    "external"
#تعریف_کرو DUK_GIT_DESCRIBE                  "external"
#تعریف_کرو DUK_GIT_BRANCH                    "external"

/* External duk_config.h provides platform/compiler/OS dependent
 * typedefs and macros, and DUK_USE_xxx config options so that
 * the rest of Duktape doesn't need to do any feature detection.
 * DUK_VERSION is defined before including so that configuration
 * snippets can react to it.
 */
#شامل "duk_config.h"

/*
 *  Avoid C++ name mangling
 */

#اگر defined(__cplusplus)
خارجی "C" {
#اگر_ختم

/*
 *  Some defines forwarded from feature detection
 */

#تعریف_منسوخ DUK_API_VARIADIC_MACROS
#اگر defined(DUK_USE_VARIADIC_MACROS)
#تعریف_کرو DUK_API_VARIADIC_MACROS
#اگر_ختم

#تعریف_کرو DUK_API_NORETURN(decl) DUK_NORETURN(decl)

/*
 *  Public API specific typedefs
 *
 *  Many types are wrapped by Duktape for portability to rare platforms
 *  where e.g. 'int' is a 16-bit type.  See practical typing discussion
 *  in Duktape web documentation.
 */

ساخت duk_thread_state;
ساخت duk_حافظہ_functions;
ساخت duk_function_list_اندراج;
ساخت duk_نمبر_list_اندراج;
ساخت duk_وقت_components;

/* duk_context is now defined in duk_config.h because it may also be
 * referenced there by prototypes.
 */
قسمتعریف ساخت duk_thread_state duk_thread_state;
قسمتعریف ساخت duk_حافظہ_functions duk_حافظہ_functions;
قسمتعریف ساخت duk_function_list_اندراج duk_function_list_اندراج;
قسمتعریف ساخت duk_نمبر_list_اندراج duk_نمبر_list_اندراج;
قسمتعریف ساخت duk_وقت_components duk_وقت_components;

قسمتعریف duk_ret_t (*duk_c_function)(duk_context *ctx);
قسمتعریف خالی *(*duk_alloc_function) (خالی *udata, duk_سائز_t سائز);
قسمتعریف خالی *(*duk_realloc_function) (خالی *udata, خالی *ptr, duk_سائز_t سائز);
قسمتعریف خالی (*duk_free_function) (خالی *udata, خالی *ptr);
قسمتعریف خالی (*duk_fatal_function) (خالی *udata, ثابت حرف *پیغام);
قسمتعریف خالی (*duk_decode_char_function) (خالی *udata, duk_codepoint_t codepoint);
قسمتعریف duk_codepoint_t (*duk_map_char_function) (خالی *udata, duk_codepoint_t codepoint);
قسمتعریف duk_ret_t (*duk_محفوظ_call_function) (duk_context *ctx, خالی *udata);
قسمتعریف duk_سائز_t (*duk_debug_read_function) (خالی *udata, حرف *بفر, duk_سائز_t لمبائی);
قسمتعریف duk_سائز_t (*duk_debug_write_function) (خالی *udata, ثابت حرف *بفر, duk_سائز_t لمبائی);
قسمتعریف duk_سائز_t (*duk_debug_peek_function) (خالی *udata);
قسمتعریف خالی (*duk_debug_read_flush_function) (خالی *udata);
قسمتعریف خالی (*duk_debug_write_flush_function) (خالی *udata);
قسمتعریف duk_idx_t (*duk_debug_request_function) (duk_context *ctx, خالی *udata, duk_idx_t nvalues);
قسمتعریف خالی (*duk_debug_detached_function) (duk_context *ctx, خالی *udata);

ساخت duk_thread_state {
	/* XXX: Enough space to hold internal suspend/resume structure.
	 * This is rather awkward and to be fixed when the internal
	 * structure is visible for the public API header.
	 */
	حرف ڈیٹا[128];
};

ساخت duk_حافظہ_functions {
	duk_alloc_function alloc_func;
	duk_realloc_function realloc_func;
	duk_free_function free_func;
	خالی *udata;
};

ساخت duk_function_list_اندراج {
	ثابت حرف *کلید;
	duk_c_function قدر;
	duk_idx_t nargs;
};

ساخت duk_نمبر_list_اندراج {
	ثابت حرف *کلید;
	duk_double_t قدر;
};

ساخت duk_وقت_components {
	duk_double_t year;          /* year, e.g. 2016, ECMAScript year range */
	duk_double_t month;         /* month: 1-12 */
	duk_double_t day;           /* day: 1-31 */
	duk_double_t hours;         /* hour: 0-59 */
	duk_double_t minutes;       /* minute: 0-59 */
	duk_double_t seconds;       /* second: 0-59 (in POSIX time no leap second) */
	duk_double_t milliseconds;  /* may contain sub-millisecond fractions */
	duk_double_t weekday;       /* weekday: 0-6, 0=Sunday, 1=Monday, ..., 6=Saturday */
};

/*
 *  Constants
 */

/* Duktape debug protocol version used by this build. */
#تعریف_کرو DUK_DEBUG_PROTOCOL_نسخہ        2

/* Used to represent invalid index; if caller uses this without checking,
 * this index will map to a non-existent stack entry.  Also used in some
 * API calls as a marker to denote "no value".
 */
#تعریف_کرو DUK_INVALID_اشاریہ                 DUK_IDX_MIN

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#تعریف_کرو DUK_VARARGS                       ((duk_int_t) (-1))

/* Number of value stack entries (in addition to actual call arguments)
 * guaranteed to be allocated on entry to a Duktape/C function.
 */
#تعریف_کرو DUK_API_اندراج_ڈھیر               64U

/* Value types, used by e.g. duk_get_type() */
#تعریف_کرو DUK_TYPE_MIN                      0U
#تعریف_کرو DUK_TYPE_NONE                     0U    /* no value, e.g. invalid index */
#تعریف_کرو DUK_TYPE_UNDEFINED                1U    /* ECMAScript undefined */
#تعریف_کرو DUK_TYPE_NULL                     2U    /* ECMAScript null */
#تعریف_کرو DUK_TYPE_BOOLEAN                  3U    /* ECMAScript boolean: 0 or 1 */
#تعریف_کرو DUK_TYPE_نمبر                   4U    /* ECMAScript number: double */
#تعریف_کرو DUK_TYPE_STRING                   5U    /* ECMAScript string: CESU-8 / extended UTF-8 encoded */
#تعریف_کرو DUK_TYPE_OBJECT                   6U    /* ECMAScript object: includes objects, arrays, functions, threads */
#تعریف_کرو DUK_TYPE_بفر                   7U    /* fixed or dynamic, garbage collected byte buffer */
#تعریف_کرو DUK_TYPE_اشارہ_گر                  8U    /* raw void pointer */
#تعریف_کرو DUK_TYPE_LIGHTFUNC                9U    /* lightweight function pointer */
#تعریف_کرو DUK_TYPE_MAX                      9U

/* Value mask types, used by e.g. duk_get_type_mask() */
#تعریف_کرو DUK_TYPE_نقاب_NONE                (1U << DUK_TYPE_NONE)
#تعریف_کرو DUK_TYPE_نقاب_UNDEFINED           (1U << DUK_TYPE_UNDEFINED)
#تعریف_کرو DUK_TYPE_نقاب_NULL                (1U << DUK_TYPE_NULL)
#تعریف_کرو DUK_TYPE_نقاب_BOOLEAN             (1U << DUK_TYPE_BOOLEAN)
#تعریف_کرو DUK_TYPE_نقاب_نمبر              (1U << DUK_TYPE_نمبر)
#تعریف_کرو DUK_TYPE_نقاب_STRING              (1U << DUK_TYPE_STRING)
#تعریف_کرو DUK_TYPE_نقاب_OBJECT              (1U << DUK_TYPE_OBJECT)
#تعریف_کرو DUK_TYPE_نقاب_بفر              (1U << DUK_TYPE_بفر)
#تعریف_کرو DUK_TYPE_نقاب_اشارہ_گر             (1U << DUK_TYPE_اشارہ_گر)
#تعریف_کرو DUK_TYPE_نقاب_LIGHTFUNC           (1U << DUK_TYPE_LIGHTFUNC)
#تعریف_کرو DUK_TYPE_نقاب_THROW               (1U << 10)  /* internal flag value: throw if mask doesn't match */
#تعریف_کرو DUK_TYPE_نقاب_PROMOTE             (1U << 11)  /* internal flag value: promote to object if mask matches */

/* Coercion hints */
#تعریف_کرو DUK_HINT_NONE                     0    /* prefer number, unless input is a Date, in which
                                                * case prefer string (E5 Section 8.12.8)
                                                */
#تعریف_کرو DUK_HINT_STRING                   1    /* prefer string */
#تعریف_کرو DUK_HINT_نمبر                   2    /* prefer number */

/* Enumeration flags for duk_enum() */
#تعریف_کرو DUK_ENUM_INCLUDE_NONENUMERABLE    (1U << 0)    /* enumerate non-numerable properties in addition to enumerable */
#تعریف_کرو DUK_ENUM_INCLUDE_HIDDEN           (1U << 1)    /* enumerate hidden symbols too (in Duktape 1.x called internal properties) */
#تعریف_کرو DUK_ENUM_INCLUDE_SYMBOLS          (1U << 2)    /* enumerate symbols */
#تعریف_کرو DUK_ENUM_EXCLUDE_لڑیاں          (1U << 3)    /* exclude strings */
#تعریف_کرو DUK_ENUM_OWN_PROPERTIES_ONLY      (1U << 4)    /* don't walk prototype chain, only check own properties */
#تعریف_کرو DUK_ENUM_ARRAY_INDICES_ONLY       (1U << 5)    /* only enumerate array indices */
/* XXX: misleading name */
#تعریف_کرو DUK_ENUM_ترتیب_دو_ARRAY_INDICES       (1U << 6)    /* sort array indices (applied to full enumeration result, including inherited array indices); XXX: misleading name */
#تعریف_کرو DUK_ENUM_NO_PROXY_BEHAVIOR        (1U << 7)    /* enumerate a proxy object itself without invoking proxy behavior */

/* Compilation flags for duk_compile() and duk_eval() */
/* DUK_COMPILE_xxx bits 0-2 are reserved for an internal 'nargs' argument.
 */
#تعریف_کرو DUK_COMPILE_EVAL                  (1U << 3)    /* compile eval code (instead of global code) */
#تعریف_کرو DUK_COMPILE_FUNCTION              (1U << 4)    /* compile function code (instead of global code) */
#تعریف_کرو DUK_COMPILE_STRICT                (1U << 5)    /* use strict (outer) context for global, eval, or function code */
#تعریف_کرو DUK_COMPILE_SHEBANG               (1U << 6)    /* allow shebang ('#! ...') comment on first line of source */
#تعریف_کرو DUK_COMPILE_محفوظ                  (1U << 7)    /* (internal) catch compilation errors */
#تعریف_کرو DUK_COMPILE_NORESULT              (1U << 8)    /* (internal) omit eval result */
#تعریف_کرو DUK_COMPILE_NOSOURCE              (1U << 9)    /* (internal) no source string on stack */
#تعریف_کرو DUK_COMPILE_STRLEN                (1U << 10)   /* (internal) take strlen() of src_buffer (avoids double evaluation in macro) */
#تعریف_کرو DUK_COMPILE_NOFILENAME            (1U << 11)   /* (internal) no filename on stack */
#تعریف_کرو DUK_COMPILE_FUNCEXPR              (1U << 12)   /* (internal) source is a function expression (used for Function constructor) */

/* Flags for duk_def_prop() and its variants; base flags + a lot of convenience shorthands */
#تعریف_کرو DUK_DEFPROP_WRITABLE              (1U << 0)    /* set writable (effective if DUK_DEFPROP_HAVE_WRITABLE set) */
#تعریف_کرو DUK_DEFPROP_ENUMERABLE            (1U << 1)    /* set enumerable (effective if DUK_DEFPROP_HAVE_ENUMERABLE set) */
#تعریف_کرو DUK_DEFPROP_CONFIGURABLE          (1U << 2)    /* set configurable (effective if DUK_DEFPROP_HAVE_CONFIGURABLE set) */
#تعریف_کرو DUK_DEFPROP_HAVE_WRITABLE         (1U << 3)    /* set/clear writable */
#تعریف_کرو DUK_DEFPROP_HAVE_ENUMERABLE       (1U << 4)    /* set/clear enumerable */
#تعریف_کرو DUK_DEFPROP_HAVE_CONFIGURABLE     (1U << 5)    /* set/clear configurable */
#تعریف_کرو DUK_DEFPROP_HAVE_قدر            (1U << 6)    /* set value (given on value stack) */
#تعریف_کرو DUK_DEFPROP_HAVE_GETTER           (1U << 7)    /* set getter (given on value stack) */
#تعریف_کرو DUK_DEFPROP_HAVE_SETTER           (1U << 8)    /* set setter (given on value stack) */
#تعریف_کرو DUK_DEFPROP_FORCE                 (1U << 9)    /* force change if possible, may still fail for e.g. virtual properties */
#تعریف_کرو DUK_DEFPROP_سیٹ_WRITABLE          (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_WRITABLE        DUK_DEFPROP_HAVE_WRITABLE
#تعریف_کرو DUK_DEFPROP_سیٹ_ENUMERABLE        (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_ENUMERABLE      DUK_DEFPROP_HAVE_ENUMERABLE
#تعریف_کرو DUK_DEFPROP_سیٹ_CONFIGURABLE      (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_CONFIGURABLE    DUK_DEFPROP_HAVE_CONFIGURABLE
#تعریف_کرو DUK_DEFPROP_W                     DUK_DEFPROP_WRITABLE
#تعریف_کرو DUK_DEFPROP_E                     DUK_DEFPROP_ENUMERABLE
#تعریف_کرو DUK_DEFPROP_C                     DUK_DEFPROP_CONFIGURABLE
#تعریف_کرو DUK_DEFPROP_WE                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE)
#تعریف_کرو DUK_DEFPROP_WC                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_EC                    (DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_WEC                   (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_HAVE_W                DUK_DEFPROP_HAVE_WRITABLE
#تعریف_کرو DUK_DEFPROP_HAVE_E                DUK_DEFPROP_HAVE_ENUMERABLE
#تعریف_کرو DUK_DEFPROP_HAVE_C                DUK_DEFPROP_HAVE_CONFIGURABLE
#تعریف_کرو DUK_DEFPROP_HAVE_WE               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE)
#تعریف_کرو DUK_DEFPROP_HAVE_WC               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_HAVE_EC               (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_HAVE_WEC              (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_سیٹ_W                 DUK_DEFPROP_سیٹ_WRITABLE
#تعریف_کرو DUK_DEFPROP_سیٹ_E                 DUK_DEFPROP_سیٹ_ENUMERABLE
#تعریف_کرو DUK_DEFPROP_سیٹ_C                 DUK_DEFPROP_سیٹ_CONFIGURABLE
#تعریف_کرو DUK_DEFPROP_سیٹ_WE                (DUK_DEFPROP_سیٹ_WRITABLE | DUK_DEFPROP_سیٹ_ENUMERABLE)
#تعریف_کرو DUK_DEFPROP_سیٹ_WC                (DUK_DEFPROP_سیٹ_WRITABLE | DUK_DEFPROP_سیٹ_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_سیٹ_EC                (DUK_DEFPROP_سیٹ_ENUMERABLE | DUK_DEFPROP_سیٹ_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_سیٹ_WEC               (DUK_DEFPROP_سیٹ_WRITABLE | DUK_DEFPROP_سیٹ_ENUMERABLE | DUK_DEFPROP_سیٹ_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_W               DUK_DEFPROP_CLEAR_WRITABLE
#تعریف_کرو DUK_DEFPROP_CLEAR_E               DUK_DEFPROP_CLEAR_ENUMERABLE
#تعریف_کرو DUK_DEFPROP_CLEAR_C               DUK_DEFPROP_CLEAR_CONFIGURABLE
#تعریف_کرو DUK_DEFPROP_CLEAR_WE              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_WC              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_EC              (DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_CLEAR_WEC             (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#تعریف_کرو DUK_DEFPROP_ATTR_W                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W)
#تعریف_کرو DUK_DEFPROP_ATTR_E                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E)
#تعریف_کرو DUK_DEFPROP_ATTR_C                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C)
#تعریف_کرو DUK_DEFPROP_ATTR_WE               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE)
#تعریف_کرو DUK_DEFPROP_ATTR_WC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC)
#تعریف_کرو DUK_DEFPROP_ATTR_EC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_EC)
#تعریف_کرو DUK_DEFPROP_ATTR_WEC              (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC)

/* Flags for duk_push_thread_raw() */
#تعریف_کرو DUK_THREAD_NEW_GLOBAL_ENV         (1U << 0)    /* create a new global environment */

/* Flags for duk_gc() */
#تعریف_کرو DUK_GC_COMPACT                    (1U << 0)    /* compact heap objects */

/* Error codes (must be 8 bits at most, see duk_error.h) */
#تعریف_کرو DUK_ERR_NONE                      0    /* no error (e.g. from duk_get_error_code()) */
#تعریف_کرو DUK_ERR_ERROR                     1    /* Error */
#تعریف_کرو DUK_ERR_EVAL_ERROR                2    /* EvalError */
#تعریف_کرو DUK_ERR_RANGE_ERROR               3    /* RangeError */
#تعریف_کرو DUK_ERR_REFERENCE_ERROR           4    /* ReferenceError */
#تعریف_کرو DUK_ERR_SYNTAX_ERROR              5    /* SyntaxError */
#تعریف_کرو DUK_ERR_TYPE_ERROR                6    /* TypeError */
#تعریف_کرو DUK_ERR_URI_ERROR                 7    /* URIError */

/* Return codes for C functions (shortcut for throwing an error) */
#تعریف_کرو DUK_RET_ERROR                     (-DUK_ERR_ERROR)
#تعریف_کرو DUK_RET_EVAL_ERROR                (-DUK_ERR_EVAL_ERROR)
#تعریف_کرو DUK_RET_RANGE_ERROR               (-DUK_ERR_RANGE_ERROR)
#تعریف_کرو DUK_RET_REFERENCE_ERROR           (-DUK_ERR_REFERENCE_ERROR)
#تعریف_کرو DUK_RET_SYNTAX_ERROR              (-DUK_ERR_SYNTAX_ERROR)
#تعریف_کرو DUK_RET_TYPE_ERROR                (-DUK_ERR_TYPE_ERROR)
#تعریف_کرو DUK_RET_URI_ERROR                 (-DUK_ERR_URI_ERROR)

/* Return codes for protected calls (duk_safe_call(), duk_pcall()) */
#تعریف_کرو DUK_EXEC_SUCCESS                  0
#تعریف_کرو DUK_EXEC_ERROR                    1

/* Debug levels for DUK_USE_DEBUG_WRITE(). */
#تعریف_کرو DUK_LEVEL_DEBUG                   0
#تعریف_کرو DUK_LEVEL_DDEBUG                  1
#تعریف_کرو DUK_LEVEL_DDDEBUG                 2

/*
 *  Macros to create Symbols as C statically constructed strings.
 *
 *  Call e.g. as DUK_HIDDEN_SYMBOL("myProperty") <=> ("\xFF" "myProperty").
 *
 *  Local symbols have a unique suffix, caller should take care to avoid
 *  conflicting with the Duktape internal representation by e.g. prepending
 *  a '!' character: DUK_LOCAL_SYMBOL("myLocal", "!123").
 *
 *  Note that these can only be used for string constants, not dynamically
 *  created strings.
 *
 *  You shouldn't normally use DUK_INTERNAL_SYMBOL() at all.  It is reserved
 *  for Duktape internal symbols only.  There are no versioning guarantees
 *  for internal symbols.
 */

#تعریف_کرو DUK_HIDDEN_علامت(x)     ("\xFF" x)
#تعریف_کرو DUK_GLOBAL_علامت(x)     ("\x80" x)
#تعریف_کرو DUK_LOCAL_علامت(x,uniq) ("\x81" x "\xff" uniq)
#تعریف_کرو DUK_WELLKNOWN_علامت(x)  ("\x81" x "\xff")
#تعریف_کرو DUK_اندرونی_علامت(x)   ("\x82" x)

/*
 *  If no variadic macros, __FILE__ and __LINE__ are passed through globals
 *  which is ugly and not thread safe.
 */

#اگر !defined(DUK_API_VARIADIC_MACROS)
DUK_EXTERNAL_DECL ثابت حرف *duk_api_global_filename;
DUK_EXTERNAL_DECL duk_int_t duk_api_global_سطر;
#اگر_ختم

/*
 *  Context management
 */

DUK_EXTERNAL_DECL
duk_context *duk_create_ڈھیر(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             خالی *ڈھیر_udata,
                             duk_fatal_function fatal_handler);
DUK_EXTERNAL_DECL خالی duk_destroy_ڈھیر(duk_context *ctx);

DUK_EXTERNAL_DECL خالی duk_suspend(duk_context *ctx, duk_thread_state *state);
DUK_EXTERNAL_DECL خالی duk_resume(duk_context *ctx, ثابت duk_thread_state *state);

#تعریف_کرو duk_create_ڈھیر_default() \
	duk_create_ڈھیر(تہی, تہی, تہی, تہی, تہی)

/*
 *  Memory management
 *
 *  Raw functions have no side effects (cannot trigger GC).
 */

DUK_EXTERNAL_DECL خالی *duk_alloc_raw(duk_context *ctx, duk_سائز_t سائز);
DUK_EXTERNAL_DECL خالی duk_free_raw(duk_context *ctx, خالی *ptr);
DUK_EXTERNAL_DECL خالی *duk_realloc_raw(duk_context *ctx, خالی *ptr, duk_سائز_t سائز);
DUK_EXTERNAL_DECL خالی *duk_alloc(duk_context *ctx, duk_سائز_t سائز);
DUK_EXTERNAL_DECL خالی duk_free(duk_context *ctx, خالی *ptr);
DUK_EXTERNAL_DECL خالی *duk_realloc(duk_context *ctx, خالی *ptr, duk_سائز_t سائز);
DUK_EXTERNAL_DECL خالی duk_get_حافظہ_functions(duk_context *ctx, duk_حافظہ_functions *برآمد_funcs);
DUK_EXTERNAL_DECL خالی duk_gc(duk_context *ctx, duk_uint_t flags);

/*
 *  Error handling
 */

DUK_API_NORETURN(DUK_EXTERNAL_DECL خالی duk_throw_raw(duk_context *ctx));
#تعریف_کرو duk_throw(ctx) \
	(duk_throw_raw((ctx)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL خالی duk_fatal_raw(duk_context *ctx, ثابت حرف *err_پیغام));
#تعریف_کرو duk_fatal(ctx,err_پیغام) \
	(duk_fatal_raw((ctx), (err_پیغام)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL خالی duk_error_raw(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *filename, duk_int_t سطر, ثابت حرف *fmt, ...));

#اگر defined(DUK_API_VARIADIC_MACROS)
#تعریف_کرو duk_error(ctx,err_رمز,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) (err_رمز), (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_generic_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_eval_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_range_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_reference_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_syntax_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_type_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#تعریف_کرو duk_uri_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#ورنہ  /* DUK_API_VARIADIC_MACROS */
/* For legacy compilers without variadic macros a macro hack is used to allow
 * variable arguments.  While the macro allows "return duk_error(...)", it
 * will fail with e.g. "(void) duk_error(...)".  The calls are noreturn but
 * with a return value to allow the "return duk_error(...)" idiom.  This may
 * cause some compiler warnings, but without noreturn the generated code is
 * often worse.  The same approach as with variadic macros (using
 * "(duk_error(...), 0)") won't work due to the macro hack structure.
 */
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_error_stash(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_generic_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_eval_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_range_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_reference_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_syntax_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_type_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_uri_error_stash(duk_context *ctx, ثابت حرف *fmt, ...));
#تعریف_کرو duk_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_error_stash)  /* last value is func pointer, arguments follow in parens */
#تعریف_کرو duk_generic_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_generic_error_stash)
#تعریف_کرو duk_eval_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_eval_error_stash)
#تعریف_کرو duk_range_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_range_error_stash)
#تعریف_کرو duk_reference_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_reference_error_stash)
#تعریف_کرو duk_syntax_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_syntax_error_stash)
#تعریف_کرو duk_type_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_type_error_stash)
#تعریف_کرو duk_uri_error  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_uri_error_stash)
#اگر_ختم  /* DUK_API_VARIADIC_MACROS */

DUK_API_NORETURN(DUK_EXTERNAL_DECL خالی duk_error_va_raw(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *filename, duk_int_t سطر, ثابت حرف *fmt, va_list ap));

#تعریف_کرو duk_error_va(ctx,err_رمز,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) (err_رمز), (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_generic_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_eval_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_range_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_reference_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_syntax_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_type_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#تعریف_کرو duk_uri_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap)), (duk_ret_t) 0)

/*
 *  Other state related functions
 */

DUK_EXTERNAL_DECL duk_bool_t duk_is_strict_call(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructor_call(duk_context *ctx);

/*
 *  Stack management
 */

DUK_EXTERNAL_DECL duk_idx_t duk_normalize_اشاریہ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_normalize_اشاریہ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_valid_اشاریہ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_require_valid_اشاریہ(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_idx_t duk_get_چوٹی(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_سیٹ_چوٹی(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_get_چوٹی_اشاریہ(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_چوٹی_اشاریہ(duk_context *ctx);

/* Although extra/top could be an unsigned type here, using a signed type
 * makes the API more robust to calling code calculation errors or corner
 * cases (where caller might occasionally come up with negative values).
 * Negative values are treated as zero, which is better than casting them
 * to a large unsigned number.  (This principle is used elsewhere in the
 * API too.)
 */
DUK_EXTERNAL_DECL duk_bool_t duk_check_ڈھیر(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL خالی duk_require_ڈھیر(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL duk_bool_t duk_check_ڈھیر_چوٹی(duk_context *ctx, duk_idx_t چوٹی);
DUK_EXTERNAL_DECL خالی duk_require_ڈھیر_چوٹی(duk_context *ctx, duk_idx_t چوٹی);

/*
 *  Stack manipulation (other than push/pop)
 */

DUK_EXTERNAL_DECL خالی duk_تبادلہ(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL خالی duk_تبادلہ_چوٹی(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_dup(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL خالی duk_dup_چوٹی(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_داخل_کرو(duk_context *ctx, duk_idx_t مقصد_idx);
DUK_EXTERNAL_DECL خالی duk_pull(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL خالی duk_replace(duk_context *ctx, duk_idx_t مقصد_idx);
DUK_EXTERNAL_DECL خالی duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t مقصد_idx);
DUK_EXTERNAL_DECL خالی duk_remove(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_xcopymove_raw(duk_context *مقصد_ctx, duk_context *from_ctx, duk_idx_t گنتی, duk_bool_t is_copy);

#تعریف_کرو duk_xmove_چوٹی(مقصد_ctx,from_ctx,گنتی) \
	duk_xcopymove_raw((مقصد_ctx), (from_ctx), (گنتی), 0 /*is_copy*/)
#تعریف_کرو duk_xcopy_چوٹی(مقصد_ctx,from_ctx,گنتی) \
	duk_xcopymove_raw((مقصد_ctx), (from_ctx), (گنتی), 1 /*is_copy*/)

/*
 *  Push operations
 *
 *  Push functions return the absolute (relative to bottom of frame)
 *  position of the pushed value for convenience.
 *
 *  Note: duk_dup() is technically a push.
 */

DUK_EXTERNAL_DECL خالی duk_ڈالو_undefined(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_null(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_boolean(duk_context *ctx, duk_bool_t val);
DUK_EXTERNAL_DECL خالی duk_ڈالو_true(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_false(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_نمبر(duk_context *ctx, duk_double_t val);
DUK_EXTERNAL_DECL خالی duk_ڈالو_nan(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_int(duk_context *ctx, duk_int_t val);
DUK_EXTERNAL_DECL خالی duk_ڈالو_uint(duk_context *ctx, duk_uint_t val);
DUK_EXTERNAL_DECL ثابت حرف *duk_ڈالو_string(duk_context *ctx, ثابت حرف *str);
DUK_EXTERNAL_DECL ثابت حرف *duk_ڈالو_lstring(duk_context *ctx, ثابت حرف *str, duk_سائز_t len);
DUK_EXTERNAL_DECL خالی duk_ڈالو_اشارہ_گر(duk_context *ctx, خالی *p);
DUK_EXTERNAL_DECL ثابت حرف *duk_ڈالو_sprintf(duk_context *ctx, ثابت حرف *fmt, ...);
DUK_EXTERNAL_DECL ثابت حرف *duk_ڈالو_vsprintf(duk_context *ctx, ثابت حرف *fmt, va_list ap);

/* duk_push_literal() may evaluate its argument (a C string literal) more than
 * once on purpose.  When speed is preferred, sizeof() avoids an unnecessary
 * strlen() at runtime.  Sizeof("foo") == 4, so subtract 1.  The argument
 * must be non-NULL and should not contain internal NUL characters as the
 * behavior will then depend on config options.
 */
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_ڈالو_literal(ctx,cstring)  duk_ڈالو_string((ctx), (cstring))
#ورنہ
DUK_EXTERNAL_DECL ثابت حرف *duk_ڈالو_literal_raw(duk_context *ctx, ثابت حرف *str, duk_سائز_t len);
#تعریف_کرو duk_ڈالو_literal(ctx,cstring)  duk_ڈالو_literal_raw((ctx), (cstring), حجم((cstring)) - 1U)
#اگر_ختم

DUK_EXTERNAL_DECL خالی duk_ڈالو_this(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_new_ہدف(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_current_function(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_current_thread(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_global_object(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_ڈھیر_stash(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_global_stash(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_ڈالو_thread_stash(duk_context *ctx, duk_context *ہدف_ctx);

DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_bare_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_bare_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_c_lightfunc(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_idx_t لمبائی, duk_int_t magic);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_thread_raw(duk_context *ctx, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_proxy(duk_context *ctx, duk_uint_t proxy_flags);

#تعریف_کرو duk_ڈالو_thread(ctx) \
	duk_ڈالو_thread_raw((ctx), 0 /*flags*/)

#تعریف_کرو duk_ڈالو_thread_new_globalenv(ctx) \
	duk_ڈالو_thread_raw((ctx), DUK_THREAD_NEW_GLOBAL_ENV /*flags*/)

DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_error_object_raw(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *filename, duk_int_t سطر, ثابت حرف *fmt, ...);

#اگر defined(DUK_API_VARIADIC_MACROS)
#تعریف_کرو duk_ڈالو_error_object(ctx,err_رمز,...)  \
	duk_ڈالو_error_object_raw((ctx), (err_رمز), (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), __VA_ARGS__)
#ورنہ
DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_error_object_stash(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *fmt, ...);
/* Note: parentheses are required so that the comma expression works in assignments. */
#تعریف_کرو duk_ڈالو_error_object  \
	(duk_api_global_filename = (ثابت حرف *) (DUK_FILE_MACRO), \
	 duk_api_global_سطر = (duk_int_t) (DUK_سطر_MACRO), \
	 duk_ڈالو_error_object_stash)  /* last value is func pointer, arguments follow in parens */
#اگر_ختم

DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_error_object_va_raw(duk_context *ctx, duk_errcode_t err_رمز, ثابت حرف *filename, duk_int_t سطر, ثابت حرف *fmt, va_list ap);
#تعریف_کرو duk_ڈالو_error_object_va(ctx,err_رمز,fmt,ap)  \
	duk_ڈالو_error_object_va_raw((ctx), (err_رمز), (ثابت حرف *) (DUK_FILE_MACRO), (duk_int_t) (DUK_سطر_MACRO), (fmt), (ap))

#تعریف_کرو DUK_BUF_FLAG_DYNAMIC   (1 << 0)    /* internal flag: dynamic buffer */
#تعریف_کرو DUK_BUF_FLAG_EXTERNAL  (1 << 1)    /* internal flag: external buffer */
#تعریف_کرو DUK_BUF_FLAG_NOZERO    (1 << 2)    /* internal flag: don't zero allocated buffer */

DUK_EXTERNAL_DECL خالی *duk_ڈالو_بفر_raw(duk_context *ctx, duk_سائز_t سائز, duk_small_uint_t flags);

#تعریف_کرو duk_ڈالو_بفر(ctx,سائز,dynamic) \
	duk_ڈالو_بفر_raw((ctx), (سائز), (dynamic) ? DUK_BUF_FLAG_DYNAMIC : 0)
#تعریف_کرو duk_ڈالو_fixed_بفر(ctx,سائز) \
	duk_ڈالو_بفر_raw((ctx), (سائز), 0 /*flags*/)
#تعریف_کرو duk_ڈالو_dynamic_بفر(ctx,سائز) \
	duk_ڈالو_بفر_raw((ctx), (سائز), DUK_BUF_FLAG_DYNAMIC /*flags*/)
#تعریف_کرو duk_ڈالو_external_بفر(ctx) \
	((خالی) duk_ڈالو_بفر_raw((ctx), 0, DUK_BUF_FLAG_DYNAMIC | DUK_BUF_FLAG_EXTERNAL))

#تعریف_کرو DUK_BUFOBJ_ARRAYBUFFER         0
#تعریف_کرو DUK_BUFOBJ_NODEJS_بفر       1
#تعریف_کرو DUK_BUFOBJ_DATAVIEW            2
#تعریف_کرو DUK_BUFOBJ_INT8ARRAY           3
#تعریف_کرو DUK_BUFOBJ_UINT8ARRAY          4
#تعریف_کرو DUK_BUFOBJ_UINT8CLAMPEDARRAY   5
#تعریف_کرو DUK_BUFOBJ_INT16ARRAY          6
#تعریف_کرو DUK_BUFOBJ_UINT16ARRAY         7
#تعریف_کرو DUK_BUFOBJ_INT32ARRAY          8
#تعریف_کرو DUK_BUFOBJ_UINT32ARRAY         9
#تعریف_کرو DUK_BUFOBJ_FLOAT32ARRAY        10
#تعریف_کرو DUK_BUFOBJ_FLOAT64ARRAY        11

DUK_EXTERNAL_DECL خالی duk_ڈالو_بفر_object(duk_context *ctx, duk_idx_t idx_بفر, duk_سائز_t byte_انحراف, duk_سائز_t byte_لمبائی, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_idx_t duk_ڈالو_heapptr(duk_context *ctx, خالی *ptr);

/*
 *  Pop operations
 */

DUK_EXTERNAL_DECL خالی duk_نکالو(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_نکالو_n(duk_context *ctx, duk_idx_t گنتی);
DUK_EXTERNAL_DECL خالی duk_نکالو_2(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_نکالو_3(duk_context *ctx);

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t idx, duk_int_t type);
DUK_EXTERNAL_DECL duk_uint_t duk_get_type_نقاب(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type_نقاب(duk_context *ctx, duk_idx_t idx, duk_uint_t نقاب);

DUK_EXTERNAL_DECL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx);
#تعریف_کرو duk_is_null_or_undefined(ctx, idx) \
	((duk_get_type_نقاب((ctx), (idx)) & (DUK_TYPE_نقاب_NULL | DUK_TYPE_نقاب_UNDEFINED)) ? 1 : 0)

DUK_EXTERNAL_DECL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_نمبر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_بفر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_بفر_ڈیٹا(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_اشارہ_گر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_علامت(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx);

#تعریف_کرو duk_is_callable(ctx,idx) \
	duk_is_function((ctx), (idx))
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructable(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_dynamic_بفر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_fixed_بفر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_external_بفر(duk_context *ctx, duk_idx_t idx);

/* Buffers and lightfuncs are not considered primitive because they mimic
 * objects and e.g. duk_to_primitive() will coerce them instead of returning
 * them as is.  Symbols are represented as strings internally.
 */
#تعریف_کرو duk_is_primitive(ctx,idx) \
	duk_check_type_نقاب((ctx), (idx), DUK_TYPE_نقاب_UNDEFINED | \
	                                  DUK_TYPE_نقاب_NULL | \
	                                  DUK_TYPE_نقاب_BOOLEAN | \
	                                  DUK_TYPE_نقاب_نمبر | \
	                                  DUK_TYPE_نقاب_STRING | \
	                                  DUK_TYPE_نقاب_اشارہ_گر)

/* Symbols are object coercible, covered by DUK_TYPE_MASK_STRING. */
#تعریف_کرو duk_is_object_coercible(ctx,idx) \
	duk_check_type_نقاب((ctx), (idx), DUK_TYPE_نقاب_BOOLEAN | \
	                                  DUK_TYPE_نقاب_نمبر | \
	                                  DUK_TYPE_نقاب_STRING | \
	                                  DUK_TYPE_نقاب_OBJECT | \
	                                  DUK_TYPE_نقاب_بفر | \
	                                  DUK_TYPE_نقاب_اشارہ_گر | \
	                                  DUK_TYPE_نقاب_LIGHTFUNC)

DUK_EXTERNAL_DECL duk_errcode_t duk_get_error_رمز(duk_context *ctx, duk_idx_t idx);
#تعریف_کرو duk_is_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) != 0)
#تعریف_کرو duk_is_eval_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_EVAL_ERROR)
#تعریف_کرو duk_is_range_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_RANGE_ERROR)
#تعریف_کرو duk_is_reference_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_REFERENCE_ERROR)
#تعریف_کرو duk_is_syntax_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_SYNTAX_ERROR)
#تعریف_کرو duk_is_type_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_TYPE_ERROR)
#تعریف_کرو duk_is_uri_error(ctx,idx) \
	(duk_get_error_رمز((ctx), (idx)) == DUK_ERR_URI_ERROR)

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_get_نمبر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_get_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len);
DUK_EXTERNAL_DECL خالی *duk_get_بفر(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز);
DUK_EXTERNAL_DECL خالی *duk_get_بفر_ڈیٹا(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز);
DUK_EXTERNAL_DECL خالی *duk_get_اشارہ_گر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_get_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی *duk_get_heapptr(duk_context *ctx, duk_idx_t idx);

/*
 *  Get-with-explicit default operations: like get operations but with an
 *  explicit default value.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean_default(duk_context *ctx, duk_idx_t idx, duk_bool_t def_قدر);
DUK_EXTERNAL_DECL duk_double_t duk_get_نمبر_default(duk_context *ctx, duk_idx_t idx, duk_double_t def_قدر);
DUK_EXTERNAL_DECL duk_int_t duk_get_int_default(duk_context *ctx, duk_idx_t idx, duk_int_t def_قدر);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint_default(duk_context *ctx, duk_idx_t idx, duk_uint_t def_قدر);
DUK_EXTERNAL_DECL ثابت حرف *duk_get_string_default(duk_context *ctx, duk_idx_t idx, ثابت حرف *def_قدر);
DUK_EXTERNAL_DECL ثابت حرف *duk_get_lstring_default(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len, ثابت حرف *def_ptr, duk_سائز_t def_len);
DUK_EXTERNAL_DECL خالی *duk_get_بفر_default(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز, خالی *def_ptr, duk_سائز_t def_len);
DUK_EXTERNAL_DECL خالی *duk_get_بفر_ڈیٹا_default(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز, خالی *def_ptr, duk_سائز_t def_len);
DUK_EXTERNAL_DECL خالی *duk_get_اشارہ_گر_default(duk_context *ctx, duk_idx_t idx, خالی *def_قدر);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function_default(duk_context *ctx, duk_idx_t idx, duk_c_function def_قدر);
DUK_EXTERNAL_DECL duk_context *duk_get_context_default(duk_context *ctx, duk_idx_t idx, duk_context *def_قدر);
DUK_EXTERNAL_DECL خالی *duk_get_heapptr_default(duk_context *ctx, duk_idx_t idx, خالی *def_قدر);

/*
 *  Opt operations: like require operations but with an explicit default value
 *  when value is undefined or index is invalid, null and non-matching types
 *  cause a TypeError.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_opt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_قدر);
DUK_EXTERNAL_DECL duk_double_t duk_opt_نمبر(duk_context *ctx, duk_idx_t idx, duk_double_t def_قدر);
DUK_EXTERNAL_DECL duk_int_t duk_opt_int(duk_context *ctx, duk_idx_t idx, duk_int_t def_قدر);
DUK_EXTERNAL_DECL duk_uint_t duk_opt_uint(duk_context *ctx, duk_idx_t idx, duk_uint_t def_قدر);
DUK_EXTERNAL_DECL ثابت حرف *duk_opt_string(duk_context *ctx, duk_idx_t idx, ثابت حرف *def_ptr);
DUK_EXTERNAL_DECL ثابت حرف *duk_opt_lstring(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len, ثابت حرف *def_ptr, duk_سائز_t def_len);
DUK_EXTERNAL_DECL خالی *duk_opt_بفر(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز, خالی *def_ptr, duk_سائز_t def_سائز);
DUK_EXTERNAL_DECL خالی *duk_opt_بفر_ڈیٹا(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز, خالی *def_ptr, duk_سائز_t def_سائز);
DUK_EXTERNAL_DECL خالی *duk_opt_اشارہ_گر(duk_context *ctx, duk_idx_t idx, خالی *def_قدر);
DUK_EXTERNAL_DECL duk_c_function duk_opt_c_function(duk_context *ctx, duk_idx_t idx, duk_c_function def_قدر);
DUK_EXTERNAL_DECL duk_context *duk_opt_context(duk_context *ctx, duk_idx_t idx, duk_context *def_قدر);
DUK_EXTERNAL_DECL خالی *duk_opt_heapptr(duk_context *ctx, duk_idx_t idx, خالی *def_قدر);

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

#تعریف_کرو duk_require_type_نقاب(ctx,idx,نقاب) \
	((خالی) duk_check_type_نقاب((ctx), (idx), (نقاب) | DUK_TYPE_نقاب_THROW))

DUK_EXTERNAL_DECL خالی duk_require_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_require_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_require_نمبر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_require_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len);
DUK_EXTERNAL_DECL خالی duk_require_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی *duk_require_بفر(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز);
DUK_EXTERNAL_DECL خالی *duk_require_بفر_ڈیٹا(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز);
DUK_EXTERNAL_DECL خالی *duk_require_اشارہ_گر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_require_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_require_function(duk_context *ctx, duk_idx_t idx);
#تعریف_کرو duk_require_callable(ctx,idx) \
	duk_require_function((ctx), (idx))
DUK_EXTERNAL_DECL خالی duk_require_constructor_call(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_require_constructable(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی *duk_require_heapptr(duk_context *ctx, duk_idx_t idx);

/* Symbols are object coercible and covered by DUK_TYPE_MASK_STRING. */
#تعریف_کرو duk_require_object_coercible(ctx,idx) \
	((خالی) duk_check_type_نقاب((ctx), (idx), DUK_TYPE_نقاب_BOOLEAN | \
	                                            DUK_TYPE_نقاب_نمبر | \
	                                            DUK_TYPE_نقاب_STRING | \
	                                            DUK_TYPE_نقاب_OBJECT | \
	                                            DUK_TYPE_نقاب_بفر | \
	                                            DUK_TYPE_نقاب_اشارہ_گر | \
	                                            DUK_TYPE_نقاب_LIGHTFUNC | \
	                                            DUK_TYPE_نقاب_THROW))

/*
 *  Coercion operations: in-place coercion, return coerced value where
 *  applicable.  If index is invalid, throw error.  Some coercions may
 *  throw an expected error (e.g. from a toString() or valueOf() call)
 *  or an internal error (e.g. from out of memory).
 */

DUK_EXTERNAL_DECL خالی duk_مقصد_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_مقصد_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_مقصد_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_مقصد_نمبر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_مقصد_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_مقصد_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int32_t duk_مقصد_int32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint32_t duk_مقصد_uint32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint16_t duk_مقصد_uint16(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_مقصد_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_مقصد_lstring(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len);
DUK_EXTERNAL_DECL خالی *duk_مقصد_بفر_raw(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز, duk_uint_t flags);
DUK_EXTERNAL_DECL خالی *duk_مقصد_اشارہ_گر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_مقصد_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_مقصد_primitive(duk_context *ctx, duk_idx_t idx, duk_int_t hint);

#تعریف_کرو DUK_BUF_منوال_FIXED      0   /* internal: request fixed buffer result */
#تعریف_کرو DUK_BUF_منوال_DYNAMIC    1   /* internal: request dynamic buffer result */
#تعریف_کرو DUK_BUF_منوال_DONTCARE   2   /* internal: don't care about fixed/dynamic nature */

#تعریف_کرو duk_مقصد_بفر(ctx,idx,برآمد_سائز) \
	duk_مقصد_بفر_raw((ctx), (idx), (برآمد_سائز), DUK_BUF_منوال_DONTCARE)
#تعریف_کرو duk_مقصد_fixed_بفر(ctx,idx,برآمد_سائز) \
	duk_مقصد_بفر_raw((ctx), (idx), (برآمد_سائز), DUK_BUF_منوال_FIXED)
#تعریف_کرو duk_مقصد_dynamic_بفر(ctx,idx,برآمد_سائز) \
	duk_مقصد_بفر_raw((ctx), (idx), (برآمد_سائز), DUK_BUF_منوال_DYNAMIC)

/* safe variants of a few coercion operations */
DUK_EXTERNAL_DECL ثابت حرف *duk_محفوظ_مقصد_lstring(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_len);
DUK_EXTERNAL_DECL ثابت حرف *duk_مقصد_stacktrace(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_محفوظ_مقصد_stacktrace(duk_context *ctx, duk_idx_t idx);
#تعریف_کرو duk_محفوظ_مقصد_string(ctx,idx) \
	duk_محفوظ_مقصد_lstring((ctx), (idx), تہی)

/*
 *  Value length
 */

DUK_EXTERNAL_DECL duk_سائز_t duk_get_لمبائی(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_سیٹ_لمبائی(duk_context *ctx, duk_idx_t idx, duk_سائز_t len);
#اگر 0
/* duk_require_length()? */
/* duk_opt_length()? */
#اگر_ختم

/*
 *  Misc conversion
 */

DUK_EXTERNAL_DECL ثابت حرف *duk_base64_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_base64_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_hex_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_hex_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ثابت حرف *duk_json_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_json_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags);
DUK_EXTERNAL_DECL خالی duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags);

DUK_EXTERNAL_DECL ثابت حرف *duk_بفر_مقصد_string(duk_context *ctx, duk_idx_t idx);

/*
 *  Buffer
 */

DUK_EXTERNAL_DECL خالی *duk_resize_بفر(duk_context *ctx, duk_idx_t idx, duk_سائز_t new_سائز);
DUK_EXTERNAL_DECL خالی *duk_steal_بفر(duk_context *ctx, duk_idx_t idx, duk_سائز_t *برآمد_سائز);
DUK_EXTERNAL_DECL خالی duk_config_بفر(duk_context *ctx, duk_idx_t idx, خالی *ptr, duk_سائز_t len);

/*
 *  Property access
 *
 *  The basic function assumes key is on stack.  The _(l)string variant takes
 *  a C string as a property name; the _literal variant takes a C literal.
 *  The _index variant takes an array index as a property name (e.g. 123 is
 *  equivalent to the key "123").  The _heapptr variant takes a raw, borrowed
 *  heap pointer.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_get_prop_literal(ctx,obj_idx,کلید)  duk_get_prop_string((ctx), (obj_idx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_get_prop_literal(ctx,obj_idx,کلید)  duk_get_prop_literal_raw((ctx), (obj_idx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_اشاریہ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, خالی *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_put_prop_literal(ctx,obj_idx,کلید)  duk_put_prop_string((ctx), (obj_idx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_put_prop_literal(ctx,obj_idx,کلید)  duk_put_prop_literal_raw((ctx), (obj_idx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_اشاریہ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, خالی *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_del_prop_literal(ctx,obj_idx,کلید)  duk_del_prop_string((ctx), (obj_idx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_del_prop_literal(ctx,obj_idx,کلید)  duk_del_prop_literal_raw((ctx), (obj_idx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_اشاریہ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, خالی *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_has_prop_literal(ctx,obj_idx,کلید)  duk_has_prop_string((ctx), (obj_idx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_has_prop_literal(ctx,obj_idx,کلید)  duk_has_prop_literal_raw((ctx), (obj_idx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_اشاریہ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, خالی *ptr);

DUK_EXTERNAL_DECL خالی duk_get_prop_desc(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);
DUK_EXTERNAL_DECL خالی duk_def_prop(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_bool_t duk_get_global_string(duk_context *ctx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_lstring(duk_context *ctx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_get_global_literal(ctx,کلید)  duk_get_global_string((ctx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_literal_raw(duk_context *ctx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_get_global_literal(ctx,کلید)  duk_get_global_literal_raw((ctx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_heapptr(duk_context *ctx, خالی *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_string(duk_context *ctx, ثابت حرف *کلید);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_lstring(duk_context *ctx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#اگر defined(DUK_USE_PREFER_سائز)
#تعریف_کرو duk_put_global_literal(ctx,کلید)  duk_put_global_string((ctx), (کلید))
#ورنہ
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_literal_raw(duk_context *ctx, ثابت حرف *کلید, duk_سائز_t کلید_len);
#تعریف_کرو duk_put_global_literal(ctx,کلید)  duk_put_global_literal_raw((ctx), (کلید), حجم((کلید)) - 1U)
#اگر_ختم
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_heapptr(duk_context *ctx, خالی *ptr);

/*
 *  Inspection
 */

DUK_EXTERNAL_DECL خالی duk_inspect_قدر(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_inspect_callstack_اندراج(duk_context *ctx, duk_int_t level);

/*
 *  Object prototype
 */

DUK_EXTERNAL_DECL خالی duk_get_prototype(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_سیٹ_prototype(duk_context *ctx, duk_idx_t idx);

/*
 *  Object finalizer
 */

DUK_EXTERNAL_DECL خالی duk_get_finalizer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_سیٹ_finalizer(duk_context *ctx, duk_idx_t idx);

/*
 *  Global object
 */

DUK_EXTERNAL_DECL خالی duk_سیٹ_global_object(duk_context *ctx);

/*
 *  Duktape/C function magic value
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_magic(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL خالی duk_سیٹ_magic(duk_context *ctx, duk_idx_t idx, duk_int_t magic);
DUK_EXTERNAL_DECL duk_int_t duk_get_current_magic(duk_context *ctx);

/*
 *  Module helpers: put multiple function or constant properties
 */

DUK_EXTERNAL_DECL خالی duk_put_function_list(duk_context *ctx, duk_idx_t obj_idx, ثابت duk_function_list_اندراج *funcs);
DUK_EXTERNAL_DECL خالی duk_put_نمبر_list(duk_context *ctx, duk_idx_t obj_idx, ثابت duk_نمبر_list_اندراج *numbers);

/*
 *  Object operations
 */

DUK_EXTERNAL_DECL خالی duk_compact(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL خالی duk_enum(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t enum_flags);
DUK_EXTERNAL_DECL duk_bool_t duk_اگلا(duk_context *ctx, duk_idx_t enum_idx, duk_bool_t get_قدر);
DUK_EXTERNAL_DECL خالی duk_seal(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL خالی duk_freeze(duk_context *ctx, duk_idx_t obj_idx);

/*
 *  String manipulation
 */

DUK_EXTERNAL_DECL خالی duk_concat(duk_context *ctx, duk_idx_t گنتی);
DUK_EXTERNAL_DECL خالی duk_join(duk_context *ctx, duk_idx_t گنتی);
DUK_EXTERNAL_DECL خالی duk_decode_string(duk_context *ctx, duk_idx_t idx, duk_decode_char_function callback, خالی *udata);
DUK_EXTERNAL_DECL خالی duk_map_string(duk_context *ctx, duk_idx_t idx, duk_map_char_function callback, خالی *udata);
DUK_EXTERNAL_DECL خالی duk_substring(duk_context *ctx, duk_idx_t idx, duk_سائز_t آغاز_char_انحراف, duk_سائز_t انتہا_char_انحراف);
DUK_EXTERNAL_DECL خالی duk_trim(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_codepoint_t duk_char_رمز_at(duk_context *ctx, duk_idx_t idx, duk_سائز_t char_انحراف);

/*
 *  ECMAScript operators
 */

DUK_EXTERNAL_DECL duk_bool_t duk_equals(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL duk_bool_t duk_strict_equals(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL duk_bool_t duk_samevalue(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL duk_bool_t duk_instanceof(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);

/*
 *  Random
 */

DUK_EXTERNAL_DECL duk_double_t duk_random(duk_context *ctx);

/*
 *  Function (method) calls
 */

DUK_EXTERNAL_DECL خالی duk_call(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL خالی duk_call_طریقہ(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL خالی duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_طریقہ(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL خالی duk_new(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_محفوظ_call(duk_context *ctx, duk_محفوظ_call_function func, خالی *udata, duk_idx_t nargs, duk_idx_t nrets);

/*
 *  Thread management
 */

/* There are currently no native functions to yield/resume, due to the internal
 * limitations on coroutine handling.  These will be added later.
 */

/*
 *  Compilation and evaluation
 */

DUK_EXTERNAL_DECL duk_int_t duk_eval_raw(duk_context *ctx, ثابت حرف *src_بفر, duk_سائز_t src_لمبائی, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_int_t duk_compile_raw(duk_context *ctx, ثابت حرف *src_بفر, duk_سائز_t src_لمبائی, duk_uint_t flags);

/* plain */
#تعریف_کرو duk_eval(ctx)  \
	((خالی) duk_eval_raw((ctx), تہی, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_eval_noresult(ctx)  \
	((خالی) duk_eval_raw((ctx), تہی, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval(ctx)  \
	(duk_eval_raw((ctx), تہی, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_محفوظ | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval_noresult(ctx)  \
	(duk_eval_raw((ctx), تہی, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_محفوظ | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_compile(ctx,flags)  \
	((خالی) duk_compile_raw((ctx), تہی, 0, 2 /*args*/ | (flags)))

#تعریف_کرو duk_pcompile(ctx,flags)  \
	(duk_compile_raw((ctx), تہی, 0, 2 /*args*/ | (flags) | DUK_COMPILE_محفوظ))

/* string */
#تعریف_کرو duk_eval_string(ctx,src)  \
	((خالی) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_eval_string_noresult(ctx,src)  \
	((خالی) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval_string(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval_string_noresult(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_compile_string(ctx,flags,src)  \
	((خالی) duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_compile_string_filename(ctx,flags,src)  \
	((خالی) duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

#تعریف_کرو duk_pcompile_string(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_pcompile_string_filename(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

/* lstring */
#تعریف_کرو duk_eval_lstring(ctx,buf,len)  \
	((خالی) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_eval_lstring_noresult(ctx,buf,len)  \
	((خالی) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval_lstring(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_محفوظ | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_peval_lstring_noresult(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_compile_lstring(ctx,flags,buf,len)  \
	((خالی) duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_compile_lstring_filename(ctx,flags,buf,len)  \
	((خالی) duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE))

#تعریف_کرو duk_pcompile_lstring(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#تعریف_کرو duk_pcompile_lstring_filename(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_محفوظ | DUK_COMPILE_NOSOURCE))

/*
 *  Bytecode load/dump
 */

DUK_EXTERNAL_DECL خالی duk_dump_function(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_بھرو_function(duk_context *ctx);

/*
 *  Debugging
 */

DUK_EXTERNAL_DECL خالی duk_ڈالو_context_dump(duk_context *ctx);

/*
 *  Debugger (debug protocol)
 */

DUK_EXTERNAL_DECL خالی duk_debugger_attach(duk_context *ctx,
                                           duk_debug_read_function read_cb,
                                           duk_debug_write_function write_cb,
                                           duk_debug_peek_function peek_cb,
                                           duk_debug_read_flush_function read_flush_cb,
                                           duk_debug_write_flush_function write_flush_cb,
                                           duk_debug_request_function request_cb,
                                           duk_debug_detached_function detached_cb,
                                           خالی *udata);
DUK_EXTERNAL_DECL خالی duk_debugger_detach(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_debugger_cooperate(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues);
DUK_EXTERNAL_DECL خالی duk_debugger_pause(duk_context *ctx);

/*
 *  Time handling
 */

DUK_EXTERNAL_DECL duk_double_t duk_get_now(duk_context *ctx);
DUK_EXTERNAL_DECL خالی duk_وقت_مقصد_components(duk_context *ctx, duk_double_t timeval, duk_وقت_components *comp);
DUK_EXTERNAL_DECL duk_double_t duk_components_مقصد_وقت(duk_context *ctx, duk_وقت_components *comp);

/*
 *  Date provider related constants
 *
 *  NOTE: These are "semi public" - you should only use these if you write
 *  your own platform specific Date provider, see doc/datetime.rst.
 */

/* Millisecond count constants. */
#تعریف_کرو DUK_DATE_MSEC_دوسرا          1000L
#تعریف_کرو DUK_DATE_MSEC_MINUTE          (60L * 1000L)
#تعریف_کرو DUK_DATE_MSEC_HOUR            (60L * 60L * 1000L)
#تعریف_کرو DUK_DATE_MSEC_DAY             (24L * 60L * 60L * 1000L)

/* ECMAScript date range is 100 million days from Epoch:
 * > 100e6 * 24 * 60 * 60 * 1000  // 100M days in millisecs
 * 8640000000000000
 * (= 8.64e15)
 */
#تعریف_کرو DUK_DATE_MSEC_100M_DAYS         (8.64e15)
#تعریف_کرو DUK_DATE_MSEC_100M_DAYS_LEEWAY  (8.64e15 + 24 * 3600e3)

/* ECMAScript year range:
 * > new Date(100e6 * 24 * 3600e3).toISOString()
 * '+275760-09-13T00:00:00.000Z'
 * > new Date(-100e6 * 24 * 3600e3).toISOString()
 * '-271821-04-20T00:00:00.000Z'
 */
#تعریف_کرو DUK_DATE_MIN_ECMA_YEAR     (-271821L)
#تعریف_کرو DUK_DATE_MAX_ECMA_YEAR     275760L

/* Part indices for internal breakdowns.  Part order from DUK_DATE_IDX_YEAR
 * to DUK_DATE_IDX_MILLISECOND matches argument ordering of ECMAScript API
 * calls (like Date constructor call).  Some functions in duk_bi_date.c
 * depend on the specific ordering, so change with care.  16 bits are not
 * enough for all parts (year, specifically).
 *
 * Must be in-sync with genbuiltins.py.
 */
#تعریف_کرو DUK_DATE_IDX_YEAR           0  /* year */
#تعریف_کرو DUK_DATE_IDX_MONTH          1  /* month: 0 to 11 */
#تعریف_کرو DUK_DATE_IDX_DAY            2  /* day within month: 0 to 30 */
#تعریف_کرو DUK_DATE_IDX_HOUR           3
#تعریف_کرو DUK_DATE_IDX_MINUTE         4
#تعریف_کرو DUK_DATE_IDX_دوسرا         5
#تعریف_کرو DUK_DATE_IDX_MILLISECOND    6
#تعریف_کرو DUK_DATE_IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#تعریف_کرو DUK_DATE_IDX_عدد_PARTS      8

/* Internal API call flags, used for various functions in duk_bi_date.c.
 * Certain flags are used by only certain functions, but since the flags
 * don't overlap, a single flags value can be passed around to multiple
 * functions.
 *
 * The unused top bits of the flags field are also used to pass values
 * to helpers (duk__get_part_helper() and duk__set_part_helper()).
 *
 * Must be in-sync with genbuiltins.py.
 */

/* NOTE: when writing a Date provider you only need a few specific
 * flags from here, the rest are internal.  Avoid using anything you
 * don't need.
 */

#تعریف_کرو DUK_DATE_FLAG_NAN_مقصد_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#تعریف_کرو DUK_DATE_FLAG_NAN_مقصد_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#تعریف_کرو DUK_DATE_FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#تعریف_کرو DUK_DATE_FLAG_EQUIVYEAR            (1 << 3)  /* timeval breakdown: replace year with equivalent year in the [1971,2037] range for DST calculations */
#تعریف_کرو DUK_DATE_FLAG_LOCALTIME            (1 << 4)  /* convert time value to local time */
#تعریف_کرو DUK_DATE_FLAG_SUB1900              (1 << 5)  /* getter: subtract 1900 from year when getting year part */
#تعریف_کرو DUK_DATE_FLAG_TOSTRING_DATE        (1 << 6)  /* include date part in string conversion result */
#تعریف_کرو DUK_DATE_FLAG_TOSTRING_وقت        (1 << 7)  /* include time part in string conversion result */
#تعریف_کرو DUK_DATE_FLAG_TOSTRING_LOCALE      (1 << 8)  /* use locale specific formatting if available */
#تعریف_کرو DUK_DATE_FLAG_TIMESETTER           (1 << 9)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#تعریف_کرو DUK_DATE_FLAG_YEAR_FIXUP           (1 << 10) /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#تعریف_کرو DUK_DATE_FLAG_SEP_T                (1 << 11) /* string conversion: use 'T' instead of ' ' as a separator */
#تعریف_کرو DUK_DATE_FLAG_قدر_سرکاؤ          12        /* additional values begin at bit 12 */

/*
 *  ROM pointer compression
 */

/* Support array for ROM pointer compression.  Only declared when ROM
 * pointer compression is active.
 */
#اگر defined(DUK_USE_ROM_OBJECTS) && defined(DUK_USE_HEAPPTR16)
DUK_EXTERNAL_DECL ثابت خالی * ثابت duk_rom_compressed_اشارے[];
#اگر_ختم

/*
 *  C++ name mangling
 */

#اگر defined(__cplusplus)
/* end 'extern "C"' wrapper */
}
#اگر_ختم

/*
 *  END PUBLIC API
 */

#اگر_ختم  /* DUKTAPE_H_INCLUDED */
