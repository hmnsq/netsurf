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

#यदि !defined(DUKTAPE_H_INCLUDED)
#परिभाषय DUKTAPE_H_INCLUDED

#परिभाषय DUK_SINGLE_FILE

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
#परिभाषय DUK_संस्करणम्                       20700L

/* Git commit, describe, and branch for Duktape build.  Useful for
 * non-official snapshot builds so that application code can easily log
 * which Duktape snapshot was used.  Not available in the ECMAScript
 * environment.
 */
#परिभाषय DUK_GIT_COMMIT                    "external"
#परिभाषय DUK_GIT_DESCRIBE                  "external"
#परिभाषय DUK_GIT_BRANCH                    "external"

/* External duk_config.h provides platform/compiler/OS dependent
 * typedefs and macros, and DUK_USE_xxx config options so that
 * the rest of Duktape doesn't need to do any feature detection.
 * DUK_VERSION is defined before including so that configuration
 * snippets can react to it.
 */
#अन्तर्भाव "duk_config.h"

/*
 *  Avoid C++ name mangling
 */

#यदि defined(__cplusplus)
बाह्य "C" {
#यदि_अन्तः

/*
 *  Some defines forwarded from feature detection
 */

#अपरिभाषय DUK_API_VARIADIC_MACROS
#यदि defined(DUK_USE_VARIADIC_MACROS)
#परिभाषय DUK_API_VARIADIC_MACROS
#यदि_अन्तः

#परिभाषय DUK_API_NORETURN(decl) DUK_NORETURN(decl)

/*
 *  Public API specific typedefs
 *
 *  Many types are wrapped by Duktape for portability to rare platforms
 *  where e.g. 'int' is a 16-bit type.  See practical typing discussion
 *  in Duktape web documentation.
 */

संरचना duk_thread_state;
संरचना duk_स्मृतिः_functions;
संरचना duk_function_list_प्रविष्टिः;
संरचना duk_सङ्ख्या_list_प्रविष्टिः;
संरचना duk_time_components;

/* duk_context is now defined in duk_config.h because it may also be
 * referenced there by prototypes.
 */
प्रकारनिर्देश संरचना duk_thread_state duk_thread_state;
प्रकारनिर्देश संरचना duk_स्मृतिः_functions duk_स्मृतिः_functions;
प्रकारनिर्देश संरचना duk_function_list_प्रविष्टिः duk_function_list_प्रविष्टिः;
प्रकारनिर्देश संरचना duk_सङ्ख्या_list_प्रविष्टिः duk_सङ्ख्या_list_प्रविष्टिः;
प्रकारनिर्देश संरचना duk_time_components duk_time_components;

प्रकारनिर्देश duk_ret_t (*duk_c_function)(duk_context *ctx);
प्रकारनिर्देश शून्य *(*duk_alloc_function) (शून्य *udata, duk_आकार_t आकार);
प्रकारनिर्देश शून्य *(*duk_realloc_function) (शून्य *udata, शून्य *ptr, duk_आकार_t आकार);
प्रकारनिर्देश शून्य (*duk_free_function) (शून्य *udata, शून्य *ptr);
प्रकारनिर्देश शून्य (*duk_fatal_function) (शून्य *udata, ध्रुव वर्ण *सन्देशः);
प्रकारनिर्देश शून्य (*duk_decode_char_function) (शून्य *udata, duk_codepoint_t codepoint);
प्रकारनिर्देश duk_codepoint_t (*duk_map_char_function) (शून्य *udata, duk_codepoint_t codepoint);
प्रकारनिर्देश duk_ret_t (*duk_safe_call_function) (duk_context *ctx, शून्य *udata);
प्रकारनिर्देश duk_आकार_t (*duk_debug_read_function) (शून्य *udata, वर्ण *बफर, duk_आकार_t दीर्घता);
प्रकारनिर्देश duk_आकार_t (*duk_debug_write_function) (शून्य *udata, ध्रुव वर्ण *बफर, duk_आकार_t दीर्घता);
प्रकारनिर्देश duk_आकार_t (*duk_debug_peek_function) (शून्य *udata);
प्रकारनिर्देश शून्य (*duk_debug_read_flush_function) (शून्य *udata);
प्रकारनिर्देश शून्य (*duk_debug_write_flush_function) (शून्य *udata);
प्रकारनिर्देश duk_idx_t (*duk_debug_request_function) (duk_context *ctx, शून्य *udata, duk_idx_t nvalues);
प्रकारनिर्देश शून्य (*duk_debug_detached_function) (duk_context *ctx, शून्य *udata);

संरचना duk_thread_state {
	/* XXX: Enough space to hold internal suspend/resume structure.
	 * This is rather awkward and to be fixed when the internal
	 * structure is visible for the public API header.
	 */
	वर्ण दत्त[128];
};

संरचना duk_स्मृतिः_functions {
	duk_alloc_function alloc_func;
	duk_realloc_function realloc_func;
	duk_free_function free_func;
	शून्य *udata;
};

संरचना duk_function_list_प्रविष्टिः {
	ध्रुव वर्ण *कुञ्चिका;
	duk_c_function मानम्;
	duk_idx_t nargs;
};

संरचना duk_सङ्ख्या_list_प्रविष्टिः {
	ध्रुव वर्ण *कुञ्चिका;
	duk_double_t मानम्;
};

संरचना duk_time_components {
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
#परिभाषय DUK_DEBUG_PROTOCOL_संस्करणम्        2

/* Used to represent invalid index; if caller uses this without checking,
 * this index will map to a non-existent stack entry.  Also used in some
 * API calls as a marker to denote "no value".
 */
#परिभाषय DUK_INVALID_सूचकांक                 DUK_IDX_MIN

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#परिभाषय DUK_VARARGS                       ((duk_int_t) (-1))

/* Number of value stack entries (in addition to actual call arguments)
 * guaranteed to be allocated on entry to a Duktape/C function.
 */
#परिभाषय DUK_API_प्रविष्टिः_राशिः               64U

/* Value types, used by e.g. duk_get_type() */
#परिभाषय DUK_TYPE_MIN                      0U
#परिभाषय DUK_TYPE_NONE                     0U    /* no value, e.g. invalid index */
#परिभाषय DUK_TYPE_UNDEFINED                1U    /* ECMAScript undefined */
#परिभाषय DUK_TYPE_NULL                     2U    /* ECMAScript null */
#परिभाषय DUK_TYPE_BOOLEAN                  3U    /* ECMAScript boolean: 0 or 1 */
#परिभाषय DUK_TYPE_सङ्ख्या                   4U    /* ECMAScript number: double */
#परिभाषय DUK_TYPE_STRING                   5U    /* ECMAScript string: CESU-8 / extended UTF-8 encoded */
#परिभाषय DUK_TYPE_OBJECT                   6U    /* ECMAScript object: includes objects, arrays, functions, threads */
#परिभाषय DUK_TYPE_बफर                   7U    /* fixed or dynamic, garbage collected byte buffer */
#परिभाषय DUK_TYPE_POINTER                  8U    /* raw void pointer */
#परिभाषय DUK_TYPE_LIGHTFUNC                9U    /* lightweight function pointer */
#परिभाषय DUK_TYPE_MAX                      9U

/* Value mask types, used by e.g. duk_get_type_mask() */
#परिभाषय DUK_TYPE_आवरणम्_NONE                (1U << DUK_TYPE_NONE)
#परिभाषय DUK_TYPE_आवरणम्_UNDEFINED           (1U << DUK_TYPE_UNDEFINED)
#परिभाषय DUK_TYPE_आवरणम्_NULL                (1U << DUK_TYPE_NULL)
#परिभाषय DUK_TYPE_आवरणम्_BOOLEAN             (1U << DUK_TYPE_BOOLEAN)
#परिभाषय DUK_TYPE_आवरणम्_सङ्ख्या              (1U << DUK_TYPE_सङ्ख्या)
#परिभाषय DUK_TYPE_आवरणम्_STRING              (1U << DUK_TYPE_STRING)
#परिभाषय DUK_TYPE_आवरणम्_OBJECT              (1U << DUK_TYPE_OBJECT)
#परिभाषय DUK_TYPE_आवरणम्_बफर              (1U << DUK_TYPE_बफर)
#परिभाषय DUK_TYPE_आवरणम्_POINTER             (1U << DUK_TYPE_POINTER)
#परिभाषय DUK_TYPE_आवरणम्_LIGHTFUNC           (1U << DUK_TYPE_LIGHTFUNC)
#परिभाषय DUK_TYPE_आवरणम्_THROW               (1U << 10)  /* internal flag value: throw if mask doesn't match */
#परिभाषय DUK_TYPE_आवरणम्_PROMOTE             (1U << 11)  /* internal flag value: promote to object if mask matches */

/* Coercion hints */
#परिभाषय DUK_HINT_NONE                     0    /* prefer number, unless input is a Date, in which
                                                * case prefer string (E5 Section 8.12.8)
                                                */
#परिभाषय DUK_HINT_STRING                   1    /* prefer string */
#परिभाषय DUK_HINT_सङ्ख्या                   2    /* prefer number */

/* Enumeration flags for duk_enum() */
#परिभाषय DUK_ENUM_INCLUDE_NONENUMERABLE    (1U << 0)    /* enumerate non-numerable properties in addition to enumerable */
#परिभाषय DUK_ENUM_INCLUDE_HIDDEN           (1U << 1)    /* enumerate hidden symbols too (in Duktape 1.x called internal properties) */
#परिभाषय DUK_ENUM_INCLUDE_SYMBOLS          (1U << 2)    /* enumerate symbols */
#परिभाषय DUK_ENUM_EXCLUDE_वाक्यानि          (1U << 3)    /* exclude strings */
#परिभाषय DUK_ENUM_OWN_PROPERTIES_ONLY      (1U << 4)    /* don't walk prototype chain, only check own properties */
#परिभाषय DUK_ENUM_ARRAY_INDICES_ONLY       (1U << 5)    /* only enumerate array indices */
/* XXX: misleading name */
#परिभाषय DUK_ENUM_क्रमय_ARRAY_INDICES       (1U << 6)    /* sort array indices (applied to full enumeration result, including inherited array indices); XXX: misleading name */
#परिभाषय DUK_ENUM_NO_PROXY_BEHAVIOR        (1U << 7)    /* enumerate a proxy object itself without invoking proxy behavior */

/* Compilation flags for duk_compile() and duk_eval() */
/* DUK_COMPILE_xxx bits 0-2 are reserved for an internal 'nargs' argument.
 */
#परिभाषय DUK_COMPILE_EVAL                  (1U << 3)    /* compile eval code (instead of global code) */
#परिभाषय DUK_COMPILE_FUNCTION              (1U << 4)    /* compile function code (instead of global code) */
#परिभाषय DUK_COMPILE_STRICT                (1U << 5)    /* use strict (outer) context for global, eval, or function code */
#परिभाषय DUK_COMPILE_SHEBANG               (1U << 6)    /* allow shebang ('#! ...') comment on first line of source */
#परिभाषय DUK_COMPILE_SAFE                  (1U << 7)    /* (internal) catch compilation errors */
#परिभाषय DUK_COMPILE_NORESULT              (1U << 8)    /* (internal) omit eval result */
#परिभाषय DUK_COMPILE_NOSOURCE              (1U << 9)    /* (internal) no source string on stack */
#परिभाषय DUK_COMPILE_STRLEN                (1U << 10)   /* (internal) take strlen() of src_buffer (avoids double evaluation in macro) */
#परिभाषय DUK_COMPILE_NOFILENAME            (1U << 11)   /* (internal) no filename on stack */
#परिभाषय DUK_COMPILE_FUNCEXPR              (1U << 12)   /* (internal) source is a function expression (used for Function constructor) */

/* Flags for duk_def_prop() and its variants; base flags + a lot of convenience shorthands */
#परिभाषय DUK_DEFPROP_WRITABLE              (1U << 0)    /* set writable (effective if DUK_DEFPROP_HAVE_WRITABLE set) */
#परिभाषय DUK_DEFPROP_ENUMERABLE            (1U << 1)    /* set enumerable (effective if DUK_DEFPROP_HAVE_ENUMERABLE set) */
#परिभाषय DUK_DEFPROP_CONFIGURABLE          (1U << 2)    /* set configurable (effective if DUK_DEFPROP_HAVE_CONFIGURABLE set) */
#परिभाषय DUK_DEFPROP_HAVE_WRITABLE         (1U << 3)    /* set/clear writable */
#परिभाषय DUK_DEFPROP_HAVE_ENUMERABLE       (1U << 4)    /* set/clear enumerable */
#परिभाषय DUK_DEFPROP_HAVE_CONFIGURABLE     (1U << 5)    /* set/clear configurable */
#परिभाषय DUK_DEFPROP_HAVE_मानम्            (1U << 6)    /* set value (given on value stack) */
#परिभाषय DUK_DEFPROP_HAVE_GETTER           (1U << 7)    /* set getter (given on value stack) */
#परिभाषय DUK_DEFPROP_HAVE_SETTER           (1U << 8)    /* set setter (given on value stack) */
#परिभाषय DUK_DEFPROP_FORCE                 (1U << 9)    /* force change if possible, may still fail for e.g. virtual properties */
#परिभाषय DUK_DEFPROP_समुच्चयः_WRITABLE          (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)
#परिभाषय DUK_DEFPROP_CLEAR_WRITABLE        DUK_DEFPROP_HAVE_WRITABLE
#परिभाषय DUK_DEFPROP_समुच्चयः_ENUMERABLE        (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)
#परिभाषय DUK_DEFPROP_CLEAR_ENUMERABLE      DUK_DEFPROP_HAVE_ENUMERABLE
#परिभाषय DUK_DEFPROP_समुच्चयः_CONFIGURABLE      (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_CLEAR_CONFIGURABLE    DUK_DEFPROP_HAVE_CONFIGURABLE
#परिभाषय DUK_DEFPROP_W                     DUK_DEFPROP_WRITABLE
#परिभाषय DUK_DEFPROP_E                     DUK_DEFPROP_ENUMERABLE
#परिभाषय DUK_DEFPROP_C                     DUK_DEFPROP_CONFIGURABLE
#परिभाषय DUK_DEFPROP_WE                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE)
#परिभाषय DUK_DEFPROP_WC                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_EC                    (DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_WEC                   (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_HAVE_W                DUK_DEFPROP_HAVE_WRITABLE
#परिभाषय DUK_DEFPROP_HAVE_E                DUK_DEFPROP_HAVE_ENUMERABLE
#परिभाषय DUK_DEFPROP_HAVE_C                DUK_DEFPROP_HAVE_CONFIGURABLE
#परिभाषय DUK_DEFPROP_HAVE_WE               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE)
#परिभाषय DUK_DEFPROP_HAVE_WC               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_HAVE_EC               (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_HAVE_WEC              (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_समुच्चयः_W                 DUK_DEFPROP_समुच्चयः_WRITABLE
#परिभाषय DUK_DEFPROP_समुच्चयः_E                 DUK_DEFPROP_समुच्चयः_ENUMERABLE
#परिभाषय DUK_DEFPROP_समुच्चयः_C                 DUK_DEFPROP_समुच्चयः_CONFIGURABLE
#परिभाषय DUK_DEFPROP_समुच्चयः_WE                (DUK_DEFPROP_समुच्चयः_WRITABLE | DUK_DEFPROP_समुच्चयः_ENUMERABLE)
#परिभाषय DUK_DEFPROP_समुच्चयः_WC                (DUK_DEFPROP_समुच्चयः_WRITABLE | DUK_DEFPROP_समुच्चयः_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_समुच्चयः_EC                (DUK_DEFPROP_समुच्चयः_ENUMERABLE | DUK_DEFPROP_समुच्चयः_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_समुच्चयः_WEC               (DUK_DEFPROP_समुच्चयः_WRITABLE | DUK_DEFPROP_समुच्चयः_ENUMERABLE | DUK_DEFPROP_समुच्चयः_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_CLEAR_W               DUK_DEFPROP_CLEAR_WRITABLE
#परिभाषय DUK_DEFPROP_CLEAR_E               DUK_DEFPROP_CLEAR_ENUMERABLE
#परिभाषय DUK_DEFPROP_CLEAR_C               DUK_DEFPROP_CLEAR_CONFIGURABLE
#परिभाषय DUK_DEFPROP_CLEAR_WE              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE)
#परिभाषय DUK_DEFPROP_CLEAR_WC              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_CLEAR_EC              (DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_CLEAR_WEC             (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#परिभाषय DUK_DEFPROP_ATTR_W                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W)
#परिभाषय DUK_DEFPROP_ATTR_E                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E)
#परिभाषय DUK_DEFPROP_ATTR_C                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C)
#परिभाषय DUK_DEFPROP_ATTR_WE               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE)
#परिभाषय DUK_DEFPROP_ATTR_WC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC)
#परिभाषय DUK_DEFPROP_ATTR_EC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_EC)
#परिभाषय DUK_DEFPROP_ATTR_WEC              (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC)

/* Flags for duk_push_thread_raw() */
#परिभाषय DUK_THREAD_NEW_GLOBAL_ENV         (1U << 0)    /* create a new global environment */

/* Flags for duk_gc() */
#परिभाषय DUK_GC_COMPACT                    (1U << 0)    /* compact heap objects */

/* Error codes (must be 8 bits at most, see duk_error.h) */
#परिभाषय DUK_ERR_NONE                      0    /* no error (e.g. from duk_get_error_code()) */
#परिभाषय DUK_ERR_ERROR                     1    /* Error */
#परिभाषय DUK_ERR_EVAL_ERROR                2    /* EvalError */
#परिभाषय DUK_ERR_RANGE_ERROR               3    /* RangeError */
#परिभाषय DUK_ERR_REFERENCE_ERROR           4    /* ReferenceError */
#परिभाषय DUK_ERR_SYNTAX_ERROR              5    /* SyntaxError */
#परिभाषय DUK_ERR_TYPE_ERROR                6    /* TypeError */
#परिभाषय DUK_ERR_URI_ERROR                 7    /* URIError */

/* Return codes for C functions (shortcut for throwing an error) */
#परिभाषय DUK_RET_ERROR                     (-DUK_ERR_ERROR)
#परिभाषय DUK_RET_EVAL_ERROR                (-DUK_ERR_EVAL_ERROR)
#परिभाषय DUK_RET_RANGE_ERROR               (-DUK_ERR_RANGE_ERROR)
#परिभाषय DUK_RET_REFERENCE_ERROR           (-DUK_ERR_REFERENCE_ERROR)
#परिभाषय DUK_RET_SYNTAX_ERROR              (-DUK_ERR_SYNTAX_ERROR)
#परिभाषय DUK_RET_TYPE_ERROR                (-DUK_ERR_TYPE_ERROR)
#परिभाषय DUK_RET_URI_ERROR                 (-DUK_ERR_URI_ERROR)

/* Return codes for protected calls (duk_safe_call(), duk_pcall()) */
#परिभाषय DUK_EXEC_SUCCESS                  0
#परिभाषय DUK_EXEC_ERROR                    1

/* Debug levels for DUK_USE_DEBUG_WRITE(). */
#परिभाषय DUK_LEVEL_DEBUG                   0
#परिभाषय DUK_LEVEL_DDEBUG                  1
#परिभाषय DUK_LEVEL_DDDEBUG                 2

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

#परिभाषय DUK_HIDDEN_चिह्नम्(x)     ("\xFF" x)
#परिभाषय DUK_GLOBAL_चिह्नम्(x)     ("\x80" x)
#परिभाषय DUK_LOCAL_चिह्नम्(x,uniq) ("\x81" x "\xff" uniq)
#परिभाषय DUK_WELLKNOWN_चिह्नम्(x)  ("\x81" x "\xff")
#परिभाषय DUK_आन्तरिकम्_चिह्नम्(x)   ("\x82" x)

/*
 *  If no variadic macros, __FILE__ and __LINE__ are passed through globals
 *  which is ugly and not thread safe.
 */

#यदि !defined(DUK_API_VARIADIC_MACROS)
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_api_global_filename;
DUK_EXTERNAL_DECL duk_int_t duk_api_global_पङ्क्तिः;
#यदि_अन्तः

/*
 *  Context management
 */

DUK_EXTERNAL_DECL
duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             शून्य *heap_udata,
                             duk_fatal_function fatal_handler);
DUK_EXTERNAL_DECL शून्य duk_destroy_heap(duk_context *ctx);

DUK_EXTERNAL_DECL शून्य duk_suspend(duk_context *ctx, duk_thread_state *state);
DUK_EXTERNAL_DECL शून्य duk_resume(duk_context *ctx, ध्रुव duk_thread_state *state);

#परिभाषय duk_create_heap_default() \
	duk_create_heap(रिक्त, रिक्त, रिक्त, रिक्त, रिक्त)

/*
 *  Memory management
 *
 *  Raw functions have no side effects (cannot trigger GC).
 */

DUK_EXTERNAL_DECL शून्य *duk_alloc_raw(duk_context *ctx, duk_आकार_t आकार);
DUK_EXTERNAL_DECL शून्य duk_free_raw(duk_context *ctx, शून्य *ptr);
DUK_EXTERNAL_DECL शून्य *duk_realloc_raw(duk_context *ctx, शून्य *ptr, duk_आकार_t आकार);
DUK_EXTERNAL_DECL शून्य *duk_alloc(duk_context *ctx, duk_आकार_t आकार);
DUK_EXTERNAL_DECL शून्य duk_free(duk_context *ctx, शून्य *ptr);
DUK_EXTERNAL_DECL शून्य *duk_realloc(duk_context *ctx, शून्य *ptr, duk_आकार_t आकार);
DUK_EXTERNAL_DECL शून्य duk_get_स्मृतिः_functions(duk_context *ctx, duk_स्मृतिः_functions *बहिर्गमनम्_funcs);
DUK_EXTERNAL_DECL शून्य duk_gc(duk_context *ctx, duk_uint_t flags);

/*
 *  Error handling
 */

DUK_API_NORETURN(DUK_EXTERNAL_DECL शून्य duk_throw_raw(duk_context *ctx));
#परिभाषय duk_throw(ctx) \
	(duk_throw_raw((ctx)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL शून्य duk_fatal_raw(duk_context *ctx, ध्रुव वर्ण *err_सन्देशः));
#परिभाषय duk_fatal(ctx,err_सन्देशः) \
	(duk_fatal_raw((ctx), (err_सन्देशः)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL शून्य duk_error_raw(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *filename, duk_int_t पङ्क्तिः, ध्रुव वर्ण *fmt, ...));

#यदि defined(DUK_API_VARIADIC_MACROS)
#परिभाषय duk_error(ctx,err_संहिता,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) (err_संहिता), (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_generic_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_eval_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_range_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_reference_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_syntax_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_type_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#परिभाषय duk_uri_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#अन्यथा  /* DUK_API_VARIADIC_MACROS */
/* For legacy compilers without variadic macros a macro hack is used to allow
 * variable arguments.  While the macro allows "return duk_error(...)", it
 * will fail with e.g. "(void) duk_error(...)".  The calls are noreturn but
 * with a return value to allow the "return duk_error(...)" idiom.  This may
 * cause some compiler warnings, but without noreturn the generated code is
 * often worse.  The same approach as with variadic macros (using
 * "(duk_error(...), 0)") won't work due to the macro hack structure.
 */
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_error_stash(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_generic_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_eval_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_range_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_reference_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_syntax_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_type_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_uri_error_stash(duk_context *ctx, ध्रुव वर्ण *fmt, ...));
#परिभाषय duk_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_error_stash)  /* last value is func pointer, arguments follow in parens */
#परिभाषय duk_generic_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_generic_error_stash)
#परिभाषय duk_eval_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_eval_error_stash)
#परिभाषय duk_range_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_range_error_stash)
#परिभाषय duk_reference_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_reference_error_stash)
#परिभाषय duk_syntax_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_syntax_error_stash)
#परिभाषय duk_type_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_type_error_stash)
#परिभाषय duk_uri_error  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_uri_error_stash)
#यदि_अन्तः  /* DUK_API_VARIADIC_MACROS */

DUK_API_NORETURN(DUK_EXTERNAL_DECL शून्य duk_error_va_raw(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *filename, duk_int_t पङ्क्तिः, ध्रुव वर्ण *fmt, va_list ap));

#परिभाषय duk_error_va(ctx,err_संहिता,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) (err_संहिता), (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_generic_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_eval_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_range_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_reference_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_syntax_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_type_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#परिभाषय duk_uri_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap)), (duk_ret_t) 0)

/*
 *  Other state related functions
 */

DUK_EXTERNAL_DECL duk_bool_t duk_is_strict_call(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructor_call(duk_context *ctx);

/*
 *  Stack management
 */

DUK_EXTERNAL_DECL duk_idx_t duk_normalize_सूचकांक(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_normalize_सूचकांक(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_valid_सूचकांक(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_require_valid_सूचकांक(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_idx_t duk_get_शिखरम्(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_शिखरम्(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_get_शिखरम्_सूचकांक(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_शिखरम्_सूचकांक(duk_context *ctx);

/* Although extra/top could be an unsigned type here, using a signed type
 * makes the API more robust to calling code calculation errors or corner
 * cases (where caller might occasionally come up with negative values).
 * Negative values are treated as zero, which is better than casting them
 * to a large unsigned number.  (This principle is used elsewhere in the
 * API too.)
 */
DUK_EXTERNAL_DECL duk_bool_t duk_check_राशिः(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL शून्य duk_require_राशिः(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL duk_bool_t duk_check_राशिः_शिखरम्(duk_context *ctx, duk_idx_t शिखरम्);
DUK_EXTERNAL_DECL शून्य duk_require_राशिः_शिखरम्(duk_context *ctx, duk_idx_t शिखरम्);

/*
 *  Stack manipulation (other than push/pop)
 */

DUK_EXTERNAL_DECL शून्य duk_swap(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL शून्य duk_swap_शिखरम्(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_dup(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL शून्य duk_dup_शिखरम्(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_निवेशय(duk_context *ctx, duk_idx_t गन्तव्यम्_idx);
DUK_EXTERNAL_DECL शून्य duk_pull(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL शून्य duk_replace(duk_context *ctx, duk_idx_t गन्तव्यम्_idx);
DUK_EXTERNAL_DECL शून्य duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t गन्तव्यम्_idx);
DUK_EXTERNAL_DECL शून्य duk_remove(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_xcopymove_raw(duk_context *गन्तव्यम्_ctx, duk_context *from_ctx, duk_idx_t गणनम्, duk_bool_t is_copy);

#परिभाषय duk_xmove_शिखरम्(गन्तव्यम्_ctx,from_ctx,गणनम्) \
	duk_xcopymove_raw((गन्तव्यम्_ctx), (from_ctx), (गणनम्), 0 /*is_copy*/)
#परिभाषय duk_xcopy_शिखरम्(गन्तव्यम्_ctx,from_ctx,गणनम्) \
	duk_xcopymove_raw((गन्तव्यम्_ctx), (from_ctx), (गणनम्), 1 /*is_copy*/)

/*
 *  Push operations
 *
 *  Push functions return the absolute (relative to bottom of frame)
 *  position of the pushed value for convenience.
 *
 *  Note: duk_dup() is technically a push.
 */

DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_undefined(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_null(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_boolean(duk_context *ctx, duk_bool_t val);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_true(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_false(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_सङ्ख्या(duk_context *ctx, duk_double_t val);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_nan(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_int(duk_context *ctx, duk_int_t val);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_uint(duk_context *ctx, duk_uint_t val);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_प्रक्षिप_string(duk_context *ctx, ध्रुव वर्ण *str);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_प्रक्षिप_lstring(duk_context *ctx, ध्रुव वर्ण *str, duk_आकार_t len);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_pointer(duk_context *ctx, शून्य *p);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_प्रक्षिप_sprintf(duk_context *ctx, ध्रुव वर्ण *fmt, ...);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_प्रक्षिप_vsprintf(duk_context *ctx, ध्रुव वर्ण *fmt, va_list ap);

/* duk_push_literal() may evaluate its argument (a C string literal) more than
 * once on purpose.  When speed is preferred, sizeof() avoids an unnecessary
 * strlen() at runtime.  Sizeof("foo") == 4, so subtract 1.  The argument
 * must be non-NULL and should not contain internal NUL characters as the
 * behavior will then depend on config options.
 */
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_प्रक्षिप_literal(ctx,cstring)  duk_प्रक्षिप_string((ctx), (cstring))
#अन्यथा
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_प्रक्षिप_literal_raw(duk_context *ctx, ध्रुव वर्ण *str, duk_आकार_t len);
#परिभाषय duk_प्रक्षिप_literal(ctx,cstring)  duk_प्रक्षिप_literal_raw((ctx), (cstring), परिमाण((cstring)) - 1U)
#यदि_अन्तः

DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_this(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_new_target(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_current_function(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_current_thread(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_global_object(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_heap_stash(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_global_stash(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_thread_stash(duk_context *ctx, duk_context *target_ctx);

DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_bare_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_bare_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_c_lightfunc(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_idx_t दीर्घता, duk_int_t magic);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_thread_raw(duk_context *ctx, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_proxy(duk_context *ctx, duk_uint_t proxy_flags);

#परिभाषय duk_प्रक्षिप_thread(ctx) \
	duk_प्रक्षिप_thread_raw((ctx), 0 /*flags*/)

#परिभाषय duk_प्रक्षिप_thread_new_globalenv(ctx) \
	duk_प्रक्षिप_thread_raw((ctx), DUK_THREAD_NEW_GLOBAL_ENV /*flags*/)

DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_error_object_raw(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *filename, duk_int_t पङ्क्तिः, ध्रुव वर्ण *fmt, ...);

#यदि defined(DUK_API_VARIADIC_MACROS)
#परिभाषय duk_प्रक्षिप_error_object(ctx,err_संहिता,...)  \
	duk_प्रक्षिप_error_object_raw((ctx), (err_संहिता), (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), __VA_ARGS__)
#अन्यथा
DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_error_object_stash(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *fmt, ...);
/* Note: parentheses are required so that the comma expression works in assignments. */
#परिभाषय duk_प्रक्षिप_error_object  \
	(duk_api_global_filename = (ध्रुव वर्ण *) (DUK_FILE_MACRO), \
	 duk_api_global_पङ्क्तिः = (duk_int_t) (DUK_पङ्क्तिः_MACRO), \
	 duk_प्रक्षिप_error_object_stash)  /* last value is func pointer, arguments follow in parens */
#यदि_अन्तः

DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_error_object_va_raw(duk_context *ctx, duk_errcode_t err_संहिता, ध्रुव वर्ण *filename, duk_int_t पङ्क्तिः, ध्रुव वर्ण *fmt, va_list ap);
#परिभाषय duk_प्रक्षिप_error_object_va(ctx,err_संहिता,fmt,ap)  \
	duk_प्रक्षिप_error_object_va_raw((ctx), (err_संहिता), (ध्रुव वर्ण *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पङ्क्तिः_MACRO), (fmt), (ap))

#परिभाषय DUK_BUF_FLAG_DYNAMIC   (1 << 0)    /* internal flag: dynamic buffer */
#परिभाषय DUK_BUF_FLAG_EXTERNAL  (1 << 1)    /* internal flag: external buffer */
#परिभाषय DUK_BUF_FLAG_NOZERO    (1 << 2)    /* internal flag: don't zero allocated buffer */

DUK_EXTERNAL_DECL शून्य *duk_प्रक्षिप_बफर_raw(duk_context *ctx, duk_आकार_t आकार, duk_small_uint_t flags);

#परिभाषय duk_प्रक्षिप_बफर(ctx,आकार,dynamic) \
	duk_प्रक्षिप_बफर_raw((ctx), (आकार), (dynamic) ? DUK_BUF_FLAG_DYNAMIC : 0)
#परिभाषय duk_प्रक्षिप_fixed_बफर(ctx,आकार) \
	duk_प्रक्षिप_बफर_raw((ctx), (आकार), 0 /*flags*/)
#परिभाषय duk_प्रक्षिप_dynamic_बफर(ctx,आकार) \
	duk_प्रक्षिप_बफर_raw((ctx), (आकार), DUK_BUF_FLAG_DYNAMIC /*flags*/)
#परिभाषय duk_प्रक्षिप_external_बफर(ctx) \
	((शून्य) duk_प्रक्षिप_बफर_raw((ctx), 0, DUK_BUF_FLAG_DYNAMIC | DUK_BUF_FLAG_EXTERNAL))

#परिभाषय DUK_BUFOBJ_ARRAYBUFFER         0
#परिभाषय DUK_BUFOBJ_NODEJS_बफर       1
#परिभाषय DUK_BUFOBJ_DATAVIEW            2
#परिभाषय DUK_BUFOBJ_INT8ARRAY           3
#परिभाषय DUK_BUFOBJ_UINT8ARRAY          4
#परिभाषय DUK_BUFOBJ_UINT8CLAMPEDARRAY   5
#परिभाषय DUK_BUFOBJ_INT16ARRAY          6
#परिभाषय DUK_BUFOBJ_UINT16ARRAY         7
#परिभाषय DUK_BUFOBJ_INT32ARRAY          8
#परिभाषय DUK_BUFOBJ_UINT32ARRAY         9
#परिभाषय DUK_BUFOBJ_FLOAT32ARRAY        10
#परिभाषय DUK_BUFOBJ_FLOAT64ARRAY        11

DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_बफर_object(duk_context *ctx, duk_idx_t idx_बफर, duk_आकार_t byte_विस्थापनम्, duk_आकार_t byte_दीर्घता, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_idx_t duk_प्रक्षिप_heapptr(duk_context *ctx, शून्य *ptr);

/*
 *  Pop operations
 */

DUK_EXTERNAL_DECL शून्य duk_निष्कासय(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_निष्कासय_n(duk_context *ctx, duk_idx_t गणनम्);
DUK_EXTERNAL_DECL शून्य duk_निष्कासय_2(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_निष्कासय_3(duk_context *ctx);

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t idx, duk_int_t type);
DUK_EXTERNAL_DECL duk_uint_t duk_get_type_आवरणम्(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type_आवरणम्(duk_context *ctx, duk_idx_t idx, duk_uint_t आवरणम्);

DUK_EXTERNAL_DECL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx);
#परिभाषय duk_is_null_or_undefined(ctx, idx) \
	((duk_get_type_आवरणम्((ctx), (idx)) & (DUK_TYPE_आवरणम्_NULL | DUK_TYPE_आवरणम्_UNDEFINED)) ? 1 : 0)

DUK_EXTERNAL_DECL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_सङ्ख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_बफर_दत्त(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_चिह्नम्(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx);

#परिभाषय duk_is_callable(ctx,idx) \
	duk_is_function((ctx), (idx))
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructable(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_dynamic_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_fixed_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_external_बफर(duk_context *ctx, duk_idx_t idx);

/* Buffers and lightfuncs are not considered primitive because they mimic
 * objects and e.g. duk_to_primitive() will coerce them instead of returning
 * them as is.  Symbols are represented as strings internally.
 */
#परिभाषय duk_is_primitive(ctx,idx) \
	duk_check_type_आवरणम्((ctx), (idx), DUK_TYPE_आवरणम्_UNDEFINED | \
	                                  DUK_TYPE_आवरणम्_NULL | \
	                                  DUK_TYPE_आवरणम्_BOOLEAN | \
	                                  DUK_TYPE_आवरणम्_सङ्ख्या | \
	                                  DUK_TYPE_आवरणम्_STRING | \
	                                  DUK_TYPE_आवरणम्_POINTER)

/* Symbols are object coercible, covered by DUK_TYPE_MASK_STRING. */
#परिभाषय duk_is_object_coercible(ctx,idx) \
	duk_check_type_आवरणम्((ctx), (idx), DUK_TYPE_आवरणम्_BOOLEAN | \
	                                  DUK_TYPE_आवरणम्_सङ्ख्या | \
	                                  DUK_TYPE_आवरणम्_STRING | \
	                                  DUK_TYPE_आवरणम्_OBJECT | \
	                                  DUK_TYPE_आवरणम्_बफर | \
	                                  DUK_TYPE_आवरणम्_POINTER | \
	                                  DUK_TYPE_आवरणम्_LIGHTFUNC)

DUK_EXTERNAL_DECL duk_errcode_t duk_get_error_संहिता(duk_context *ctx, duk_idx_t idx);
#परिभाषय duk_is_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) != 0)
#परिभाषय duk_is_eval_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_EVAL_ERROR)
#परिभाषय duk_is_range_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_RANGE_ERROR)
#परिभाषय duk_is_reference_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_REFERENCE_ERROR)
#परिभाषय duk_is_syntax_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_SYNTAX_ERROR)
#परिभाषय duk_is_type_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_TYPE_ERROR)
#परिभाषय duk_is_uri_error(ctx,idx) \
	(duk_get_error_संहिता((ctx), (idx)) == DUK_ERR_URI_ERROR)

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_get_सङ्ख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_get_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len);
DUK_EXTERNAL_DECL शून्य *duk_get_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार);
DUK_EXTERNAL_DECL शून्य *duk_get_बफर_दत्त(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार);
DUK_EXTERNAL_DECL शून्य *duk_get_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_get_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य *duk_get_heapptr(duk_context *ctx, duk_idx_t idx);

/*
 *  Get-with-explicit default operations: like get operations but with an
 *  explicit default value.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean_default(duk_context *ctx, duk_idx_t idx, duk_bool_t def_मानम्);
DUK_EXTERNAL_DECL duk_double_t duk_get_सङ्ख्या_default(duk_context *ctx, duk_idx_t idx, duk_double_t def_मानम्);
DUK_EXTERNAL_DECL duk_int_t duk_get_int_default(duk_context *ctx, duk_idx_t idx, duk_int_t def_मानम्);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint_default(duk_context *ctx, duk_idx_t idx, duk_uint_t def_मानम्);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_get_string_default(duk_context *ctx, duk_idx_t idx, ध्रुव वर्ण *def_मानम्);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_get_lstring_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len, ध्रुव वर्ण *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL शून्य *duk_get_बफर_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार, शून्य *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL शून्य *duk_get_बफर_दत्त_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार, शून्य *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL शून्य *duk_get_pointer_default(duk_context *ctx, duk_idx_t idx, शून्य *def_मानम्);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function_default(duk_context *ctx, duk_idx_t idx, duk_c_function def_मानम्);
DUK_EXTERNAL_DECL duk_context *duk_get_context_default(duk_context *ctx, duk_idx_t idx, duk_context *def_मानम्);
DUK_EXTERNAL_DECL शून्य *duk_get_heapptr_default(duk_context *ctx, duk_idx_t idx, शून्य *def_मानम्);

/*
 *  Opt operations: like require operations but with an explicit default value
 *  when value is undefined or index is invalid, null and non-matching types
 *  cause a TypeError.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_opt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_मानम्);
DUK_EXTERNAL_DECL duk_double_t duk_opt_सङ्ख्या(duk_context *ctx, duk_idx_t idx, duk_double_t def_मानम्);
DUK_EXTERNAL_DECL duk_int_t duk_opt_int(duk_context *ctx, duk_idx_t idx, duk_int_t def_मानम्);
DUK_EXTERNAL_DECL duk_uint_t duk_opt_uint(duk_context *ctx, duk_idx_t idx, duk_uint_t def_मानम्);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_opt_string(duk_context *ctx, duk_idx_t idx, ध्रुव वर्ण *def_ptr);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_opt_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len, ध्रुव वर्ण *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL शून्य *duk_opt_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार, शून्य *def_ptr, duk_आकार_t def_आकार);
DUK_EXTERNAL_DECL शून्य *duk_opt_बफर_दत्त(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार, शून्य *def_ptr, duk_आकार_t def_आकार);
DUK_EXTERNAL_DECL शून्य *duk_opt_pointer(duk_context *ctx, duk_idx_t idx, शून्य *def_मानम्);
DUK_EXTERNAL_DECL duk_c_function duk_opt_c_function(duk_context *ctx, duk_idx_t idx, duk_c_function def_मानम्);
DUK_EXTERNAL_DECL duk_context *duk_opt_context(duk_context *ctx, duk_idx_t idx, duk_context *def_मानम्);
DUK_EXTERNAL_DECL शून्य *duk_opt_heapptr(duk_context *ctx, duk_idx_t idx, शून्य *def_मानम्);

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

#परिभाषय duk_require_type_आवरणम्(ctx,idx,आवरणम्) \
	((शून्य) duk_check_type_आवरणम्((ctx), (idx), (आवरणम्) | DUK_TYPE_आवरणम्_THROW))

DUK_EXTERNAL_DECL शून्य duk_require_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_require_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_require_सङ्ख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_require_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len);
DUK_EXTERNAL_DECL शून्य duk_require_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य *duk_require_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार);
DUK_EXTERNAL_DECL शून्य *duk_require_बफर_दत्त(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार);
DUK_EXTERNAL_DECL शून्य *duk_require_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_require_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_require_function(duk_context *ctx, duk_idx_t idx);
#परिभाषय duk_require_callable(ctx,idx) \
	duk_require_function((ctx), (idx))
DUK_EXTERNAL_DECL शून्य duk_require_constructor_call(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_require_constructable(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य *duk_require_heapptr(duk_context *ctx, duk_idx_t idx);

/* Symbols are object coercible and covered by DUK_TYPE_MASK_STRING. */
#परिभाषय duk_require_object_coercible(ctx,idx) \
	((शून्य) duk_check_type_आवरणम्((ctx), (idx), DUK_TYPE_आवरणम्_BOOLEAN | \
	                                            DUK_TYPE_आवरणम्_सङ्ख्या | \
	                                            DUK_TYPE_आवरणम्_STRING | \
	                                            DUK_TYPE_आवरणम्_OBJECT | \
	                                            DUK_TYPE_आवरणम्_बफर | \
	                                            DUK_TYPE_आवरणम्_POINTER | \
	                                            DUK_TYPE_आवरणम्_LIGHTFUNC | \
	                                            DUK_TYPE_आवरणम्_THROW))

/*
 *  Coercion operations: in-place coercion, return coerced value where
 *  applicable.  If index is invalid, throw error.  Some coercions may
 *  throw an expected error (e.g. from a toString() or valueOf() call)
 *  or an internal error (e.g. from out of memory).
 */

DUK_EXTERNAL_DECL शून्य duk_गन्तव्यम्_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_गन्तव्यम्_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_गन्तव्यम्_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_गन्तव्यम्_सङ्ख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_गन्तव्यम्_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_गन्तव्यम्_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int32_t duk_गन्तव्यम्_int32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint32_t duk_गन्तव्यम्_uint32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint16_t duk_गन्तव्यम्_uint16(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_गन्तव्यम्_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_गन्तव्यम्_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len);
DUK_EXTERNAL_DECL शून्य *duk_गन्तव्यम्_बफर_raw(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार, duk_uint_t flags);
DUK_EXTERNAL_DECL शून्य *duk_गन्तव्यम्_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_गन्तव्यम्_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_गन्तव्यम्_primitive(duk_context *ctx, duk_idx_t idx, duk_int_t hint);

#परिभाषय DUK_BUF_बहुलकम्_FIXED      0   /* internal: request fixed buffer result */
#परिभाषय DUK_BUF_बहुलकम्_DYNAMIC    1   /* internal: request dynamic buffer result */
#परिभाषय DUK_BUF_बहुलकम्_DONTCARE   2   /* internal: don't care about fixed/dynamic nature */

#परिभाषय duk_गन्तव्यम्_बफर(ctx,idx,बहिर्गमनम्_आकार) \
	duk_गन्तव्यम्_बफर_raw((ctx), (idx), (बहिर्गमनम्_आकार), DUK_BUF_बहुलकम्_DONTCARE)
#परिभाषय duk_गन्तव्यम्_fixed_बफर(ctx,idx,बहिर्गमनम्_आकार) \
	duk_गन्तव्यम्_बफर_raw((ctx), (idx), (बहिर्गमनम्_आकार), DUK_BUF_बहुलकम्_FIXED)
#परिभाषय duk_गन्तव्यम्_dynamic_बफर(ctx,idx,बहिर्गमनम्_आकार) \
	duk_गन्तव्यम्_बफर_raw((ctx), (idx), (बहिर्गमनम्_आकार), DUK_BUF_बहुलकम्_DYNAMIC)

/* safe variants of a few coercion operations */
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_safe_गन्तव्यम्_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_len);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_गन्तव्यम्_stacktrace(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_safe_गन्तव्यम्_stacktrace(duk_context *ctx, duk_idx_t idx);
#परिभाषय duk_safe_गन्तव्यम्_string(ctx,idx) \
	duk_safe_गन्तव्यम्_lstring((ctx), (idx), रिक्त)

/*
 *  Value length
 */

DUK_EXTERNAL_DECL duk_आकार_t duk_get_दीर्घता(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_दीर्घता(duk_context *ctx, duk_idx_t idx, duk_आकार_t len);
#यदि 0
/* duk_require_length()? */
/* duk_opt_length()? */
#यदि_अन्तः

/*
 *  Misc conversion
 */

DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_base64_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_base64_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_hex_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_hex_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_json_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_json_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags);
DUK_EXTERNAL_DECL शून्य duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags);

DUK_EXTERNAL_DECL ध्रुव वर्ण *duk_बफर_गन्तव्यम्_string(duk_context *ctx, duk_idx_t idx);

/*
 *  Buffer
 */

DUK_EXTERNAL_DECL शून्य *duk_resize_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t new_आकार);
DUK_EXTERNAL_DECL शून्य *duk_steal_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *बहिर्गमनम्_आकार);
DUK_EXTERNAL_DECL शून्य duk_config_बफर(duk_context *ctx, duk_idx_t idx, शून्य *ptr, duk_आकार_t len);

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
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_get_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_get_prop_string((ctx), (obj_idx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_get_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_get_prop_literal_raw((ctx), (obj_idx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, शून्य *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_put_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_put_prop_string((ctx), (obj_idx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_put_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_put_prop_literal_raw((ctx), (obj_idx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, शून्य *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_del_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_del_prop_string((ctx), (obj_idx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_del_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_del_prop_literal_raw((ctx), (obj_idx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, शून्य *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_has_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_has_prop_string((ctx), (obj_idx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_has_prop_literal(ctx,obj_idx,कुञ्चिका)  duk_has_prop_literal_raw((ctx), (obj_idx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, शून्य *ptr);

DUK_EXTERNAL_DECL शून्य duk_get_prop_desc(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);
DUK_EXTERNAL_DECL शून्य duk_def_prop(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_bool_t duk_get_global_string(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_lstring(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_get_global_literal(ctx,कुञ्चिका)  duk_get_global_string((ctx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_literal_raw(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_get_global_literal(ctx,कुञ्चिका)  duk_get_global_literal_raw((ctx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_heapptr(duk_context *ctx, शून्य *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_string(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_lstring(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#यदि defined(DUK_USE_PREFER_आकार)
#परिभाषय duk_put_global_literal(ctx,कुञ्चिका)  duk_put_global_string((ctx), (कुञ्चिका))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_literal_raw(duk_context *ctx, ध्रुव वर्ण *कुञ्चिका, duk_आकार_t कुञ्चिका_len);
#परिभाषय duk_put_global_literal(ctx,कुञ्चिका)  duk_put_global_literal_raw((ctx), (कुञ्चिका), परिमाण((कुञ्चिका)) - 1U)
#यदि_अन्तः
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_heapptr(duk_context *ctx, शून्य *ptr);

/*
 *  Inspection
 */

DUK_EXTERNAL_DECL शून्य duk_inspect_मानम्(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_inspect_callstack_प्रविष्टिः(duk_context *ctx, duk_int_t level);

/*
 *  Object prototype
 */

DUK_EXTERNAL_DECL शून्य duk_get_prototype(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_prototype(duk_context *ctx, duk_idx_t idx);

/*
 *  Object finalizer
 */

DUK_EXTERNAL_DECL शून्य duk_get_finalizer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_finalizer(duk_context *ctx, duk_idx_t idx);

/*
 *  Global object
 */

DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_global_object(duk_context *ctx);

/*
 *  Duktape/C function magic value
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_magic(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL शून्य duk_समुच्चयः_magic(duk_context *ctx, duk_idx_t idx, duk_int_t magic);
DUK_EXTERNAL_DECL duk_int_t duk_get_current_magic(duk_context *ctx);

/*
 *  Module helpers: put multiple function or constant properties
 */

DUK_EXTERNAL_DECL शून्य duk_put_function_list(duk_context *ctx, duk_idx_t obj_idx, ध्रुव duk_function_list_प्रविष्टिः *funcs);
DUK_EXTERNAL_DECL शून्य duk_put_सङ्ख्या_list(duk_context *ctx, duk_idx_t obj_idx, ध्रुव duk_सङ्ख्या_list_प्रविष्टिः *numbers);

/*
 *  Object operations
 */

DUK_EXTERNAL_DECL शून्य duk_compact(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL शून्य duk_enum(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t enum_flags);
DUK_EXTERNAL_DECL duk_bool_t duk_अग्र(duk_context *ctx, duk_idx_t enum_idx, duk_bool_t get_मानम्);
DUK_EXTERNAL_DECL शून्य duk_seal(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL शून्य duk_freeze(duk_context *ctx, duk_idx_t obj_idx);

/*
 *  String manipulation
 */

DUK_EXTERNAL_DECL शून्य duk_concat(duk_context *ctx, duk_idx_t गणनम्);
DUK_EXTERNAL_DECL शून्य duk_join(duk_context *ctx, duk_idx_t गणनम्);
DUK_EXTERNAL_DECL शून्य duk_decode_string(duk_context *ctx, duk_idx_t idx, duk_decode_char_function callback, शून्य *udata);
DUK_EXTERNAL_DECL शून्य duk_map_string(duk_context *ctx, duk_idx_t idx, duk_map_char_function callback, शून्य *udata);
DUK_EXTERNAL_DECL शून्य duk_substring(duk_context *ctx, duk_idx_t idx, duk_आकार_t start_char_विस्थापनम्, duk_आकार_t अन्तः_char_विस्थापनम्);
DUK_EXTERNAL_DECL शून्य duk_trim(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_codepoint_t duk_char_संहिता_at(duk_context *ctx, duk_idx_t idx, duk_आकार_t char_विस्थापनम्);

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

DUK_EXTERNAL_DECL शून्य duk_call(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL शून्य duk_call_विधानम्(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL शून्य duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_विधानम्(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL शून्य duk_new(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, शून्य *udata, duk_idx_t nargs, duk_idx_t nrets);

/*
 *  Thread management
 */

/* There are currently no native functions to yield/resume, due to the internal
 * limitations on coroutine handling.  These will be added later.
 */

/*
 *  Compilation and evaluation
 */

DUK_EXTERNAL_DECL duk_int_t duk_eval_raw(duk_context *ctx, ध्रुव वर्ण *src_बफर, duk_आकार_t src_दीर्घता, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_int_t duk_compile_raw(duk_context *ctx, ध्रुव वर्ण *src_बफर, duk_आकार_t src_दीर्घता, duk_uint_t flags);

/* plain */
#परिभाषय duk_eval(ctx)  \
	((शून्य) duk_eval_raw((ctx), रिक्त, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_eval_noresult(ctx)  \
	((शून्य) duk_eval_raw((ctx), रिक्त, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval(ctx)  \
	(duk_eval_raw((ctx), रिक्त, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval_noresult(ctx)  \
	(duk_eval_raw((ctx), रिक्त, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_compile(ctx,flags)  \
	((शून्य) duk_compile_raw((ctx), रिक्त, 0, 2 /*args*/ | (flags)))

#परिभाषय duk_pcompile(ctx,flags)  \
	(duk_compile_raw((ctx), रिक्त, 0, 2 /*args*/ | (flags) | DUK_COMPILE_SAFE))

/* string */
#परिभाषय duk_eval_string(ctx,src)  \
	((शून्य) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_eval_string_noresult(ctx,src)  \
	((शून्य) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval_string(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval_string_noresult(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_compile_string(ctx,flags,src)  \
	((शून्य) duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_compile_string_filename(ctx,flags,src)  \
	((शून्य) duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

#परिभाषय duk_pcompile_string(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_pcompile_string_filename(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

/* lstring */
#परिभाषय duk_eval_lstring(ctx,buf,len)  \
	((शून्य) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_eval_lstring_noresult(ctx,buf,len)  \
	((शून्य) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval_lstring(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_SAFE | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_peval_lstring_noresult(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_compile_lstring(ctx,flags,buf,len)  \
	((शून्य) duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_compile_lstring_filename(ctx,flags,buf,len)  \
	((शून्य) duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE))

#परिभाषय duk_pcompile_lstring(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#परिभाषय duk_pcompile_lstring_filename(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE))

/*
 *  Bytecode load/dump
 */

DUK_EXTERNAL_DECL शून्य duk_dump_function(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_आरोपय_function(duk_context *ctx);

/*
 *  Debugging
 */

DUK_EXTERNAL_DECL शून्य duk_प्रक्षिप_context_dump(duk_context *ctx);

/*
 *  Debugger (debug protocol)
 */

DUK_EXTERNAL_DECL शून्य duk_debugger_attach(duk_context *ctx,
                                           duk_debug_read_function read_cb,
                                           duk_debug_write_function write_cb,
                                           duk_debug_peek_function peek_cb,
                                           duk_debug_read_flush_function read_flush_cb,
                                           duk_debug_write_flush_function write_flush_cb,
                                           duk_debug_request_function request_cb,
                                           duk_debug_detached_function detached_cb,
                                           शून्य *udata);
DUK_EXTERNAL_DECL शून्य duk_debugger_detach(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_debugger_cooperate(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues);
DUK_EXTERNAL_DECL शून्य duk_debugger_pause(duk_context *ctx);

/*
 *  Time handling
 */

DUK_EXTERNAL_DECL duk_double_t duk_get_now(duk_context *ctx);
DUK_EXTERNAL_DECL शून्य duk_time_गन्तव्यम्_components(duk_context *ctx, duk_double_t timeval, duk_time_components *comp);
DUK_EXTERNAL_DECL duk_double_t duk_components_गन्तव्यम्_time(duk_context *ctx, duk_time_components *comp);

/*
 *  Date provider related constants
 *
 *  NOTE: These are "semi public" - you should only use these if you write
 *  your own platform specific Date provider, see doc/datetime.rst.
 */

/* Millisecond count constants. */
#परिभाषय DUK_DATE_MSEC_SECOND          1000L
#परिभाषय DUK_DATE_MSEC_MINUTE          (60L * 1000L)
#परिभाषय DUK_DATE_MSEC_HOUR            (60L * 60L * 1000L)
#परिभाषय DUK_DATE_MSEC_DAY             (24L * 60L * 60L * 1000L)

/* ECMAScript date range is 100 million days from Epoch:
 * > 100e6 * 24 * 60 * 60 * 1000  // 100M days in millisecs
 * 8640000000000000
 * (= 8.64e15)
 */
#परिभाषय DUK_DATE_MSEC_100M_DAYS         (8.64e15)
#परिभाषय DUK_DATE_MSEC_100M_DAYS_LEEWAY  (8.64e15 + 24 * 3600e3)

/* ECMAScript year range:
 * > new Date(100e6 * 24 * 3600e3).toISOString()
 * '+275760-09-13T00:00:00.000Z'
 * > new Date(-100e6 * 24 * 3600e3).toISOString()
 * '-271821-04-20T00:00:00.000Z'
 */
#परिभाषय DUK_DATE_MIN_ECMA_YEAR     (-271821L)
#परिभाषय DUK_DATE_MAX_ECMA_YEAR     275760L

/* Part indices for internal breakdowns.  Part order from DUK_DATE_IDX_YEAR
 * to DUK_DATE_IDX_MILLISECOND matches argument ordering of ECMAScript API
 * calls (like Date constructor call).  Some functions in duk_bi_date.c
 * depend on the specific ordering, so change with care.  16 bits are not
 * enough for all parts (year, specifically).
 *
 * Must be in-sync with genbuiltins.py.
 */
#परिभाषय DUK_DATE_IDX_YEAR           0  /* year */
#परिभाषय DUK_DATE_IDX_MONTH          1  /* month: 0 to 11 */
#परिभाषय DUK_DATE_IDX_DAY            2  /* day within month: 0 to 30 */
#परिभाषय DUK_DATE_IDX_HOUR           3
#परिभाषय DUK_DATE_IDX_MINUTE         4
#परिभाषय DUK_DATE_IDX_SECOND         5
#परिभाषय DUK_DATE_IDX_MILLISECOND    6
#परिभाषय DUK_DATE_IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#परिभाषय DUK_DATE_IDX_अङ्कः_PARTS      8

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

#परिभाषय DUK_DATE_FLAG_NAN_गन्तव्यम्_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#परिभाषय DUK_DATE_FLAG_NAN_गन्तव्यम्_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#परिभाषय DUK_DATE_FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#परिभाषय DUK_DATE_FLAG_EQUIVYEAR            (1 << 3)  /* timeval breakdown: replace year with equivalent year in the [1971,2037] range for DST calculations */
#परिभाषय DUK_DATE_FLAG_LOCALTIME            (1 << 4)  /* convert time value to local time */
#परिभाषय DUK_DATE_FLAG_SUB1900              (1 << 5)  /* getter: subtract 1900 from year when getting year part */
#परिभाषय DUK_DATE_FLAG_TOSTRING_DATE        (1 << 6)  /* include date part in string conversion result */
#परिभाषय DUK_DATE_FLAG_TOSTRING_TIME        (1 << 7)  /* include time part in string conversion result */
#परिभाषय DUK_DATE_FLAG_TOSTRING_LOCALE      (1 << 8)  /* use locale specific formatting if available */
#परिभाषय DUK_DATE_FLAG_TIMESETTER           (1 << 9)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#परिभाषय DUK_DATE_FLAG_YEAR_FIXUP           (1 << 10) /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#परिभाषय DUK_DATE_FLAG_SEP_T                (1 << 11) /* string conversion: use 'T' instead of ' ' as a separator */
#परिभाषय DUK_DATE_FLAG_मानम्_सरणम्          12        /* additional values begin at bit 12 */

/*
 *  ROM pointer compression
 */

/* Support array for ROM pointer compression.  Only declared when ROM
 * pointer compression is active.
 */
#यदि defined(DUK_USE_ROM_OBJECTS) && defined(DUK_USE_HEAPPTR16)
DUK_EXTERNAL_DECL ध्रुव शून्य * ध्रुव duk_rom_compressed_सूचकाः[];
#यदि_अन्तः

/*
 *  C++ name mangling
 */

#यदि defined(__cplusplus)
/* end 'extern "C"' wrapper */
}
#यदि_अन्तः

/*
 *  END PUBLIC API
 */

#यदि_अन्तः  /* DUKTAPE_H_INCLUDED */
