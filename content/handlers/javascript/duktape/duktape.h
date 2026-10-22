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

#अगर !defined(DUKTAPE_H_INCLUDED)
#घोषणा DUKTAPE_H_INCLUDED

#घोषणा DUK_SINGLE_FILE

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
#घोषणा DUK_VERSION                       20700L

/* Git commit, describe, and branch for Duktape build.  Useful for
 * non-official snapshot builds so that application code can easily log
 * which Duktape snapshot was used.  Not available in the ECMAScript
 * environment.
 */
#घोषणा DUK_GIT_COMMIT                    "external"
#घोषणा DUK_GIT_DESCRIBE                  "external"
#घोषणा DUK_GIT_BRANCH                    "external"

/* External duk_config.h provides platform/compiler/OS dependent
 * typedefs and macros, and DUK_USE_xxx config options so that
 * the rest of Duktape doesn't need to do any feature detection.
 * DUK_VERSION is defined before including so that configuration
 * snippets can react to it.
 */
#समावेश "duk_config.h"

/*
 *  Avoid C++ name mangling
 */

#अगर defined(__cplusplus)
बाह्य "C" {
#पूर्ण_अगर

/*
 *  Some defines forwarded from feature detection
 */

#अघोषित DUK_API_VARIADIC_MACROS
#अगर defined(DUK_USE_VARIADIC_MACROS)
#घोषणा DUK_API_VARIADIC_MACROS
#पूर्ण_अगर

#घोषणा DUK_API_NORETURN(decl) DUK_NORETURN(decl)

/*
 *  Public API specific typedefs
 *
 *  Many types are wrapped by Duktape for portability to rare platforms
 *  where e.g. 'int' is a 16-bit type.  See practical typing discussion
 *  in Duktape web documentation.
 */

काष्ठा duk_thread_state;
काष्ठा duk_memory_functions;
काष्ठा duk_function_list_entry;
काष्ठा duk_संख्या_list_entry;
काष्ठा duk_time_components;

/* duk_context is now defined in duk_config.h because it may also be
 * referenced there by prototypes.
 */
प्रकार काष्ठा duk_thread_state duk_thread_state;
प्रकार काष्ठा duk_memory_functions duk_memory_functions;
प्रकार काष्ठा duk_function_list_entry duk_function_list_entry;
प्रकार काष्ठा duk_संख्या_list_entry duk_संख्या_list_entry;
प्रकार काष्ठा duk_time_components duk_time_components;

प्रकार duk_ret_t (*duk_c_function)(duk_context *ctx);
प्रकार व्योम *(*duk_alloc_function) (व्योम *udata, duk_आकार_t आकार);
प्रकार व्योम *(*duk_realloc_function) (व्योम *udata, व्योम *ptr, duk_आकार_t आकार);
प्रकार व्योम (*duk_free_function) (व्योम *udata, व्योम *ptr);
प्रकार व्योम (*duk_fatal_function) (व्योम *udata, स्थिर अक्षर *संदेश);
प्रकार व्योम (*duk_decode_char_function) (व्योम *udata, duk_codepoint_t codepoint);
प्रकार duk_codepoint_t (*duk_map_char_function) (व्योम *udata, duk_codepoint_t codepoint);
प्रकार duk_ret_t (*duk_safe_call_function) (duk_context *ctx, व्योम *udata);
प्रकार duk_आकार_t (*duk_debug_read_function) (व्योम *udata, अक्षर *बफर, duk_आकार_t लंबाई);
प्रकार duk_आकार_t (*duk_debug_write_function) (व्योम *udata, स्थिर अक्षर *बफर, duk_आकार_t लंबाई);
प्रकार duk_आकार_t (*duk_debug_peek_function) (व्योम *udata);
प्रकार व्योम (*duk_debug_read_flush_function) (व्योम *udata);
प्रकार व्योम (*duk_debug_write_flush_function) (व्योम *udata);
प्रकार duk_idx_t (*duk_debug_request_function) (duk_context *ctx, व्योम *udata, duk_idx_t nvalues);
प्रकार व्योम (*duk_debug_detached_function) (duk_context *ctx, व्योम *udata);

काष्ठा duk_thread_state {
	/* XXX: Enough space to hold internal suspend/resume structure.
	 * This is rather awkward and to be fixed when the internal
	 * structure is visible for the public API header.
	 */
	अक्षर तथ्य[128];
};

काष्ठा duk_memory_functions {
	duk_alloc_function alloc_func;
	duk_realloc_function realloc_func;
	duk_free_function free_func;
	व्योम *udata;
};

काष्ठा duk_function_list_entry {
	स्थिर अक्षर *कुंजी;
	duk_c_function मूल्य;
	duk_idx_t nargs;
};

काष्ठा duk_संख्या_list_entry {
	स्थिर अक्षर *कुंजी;
	duk_double_t मूल्य;
};

काष्ठा duk_time_components {
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
#घोषणा DUK_DEBUG_PROTOCOL_VERSION        2

/* Used to represent invalid index; if caller uses this without checking,
 * this index will map to a non-existent stack entry.  Also used in some
 * API calls as a marker to denote "no value".
 */
#घोषणा DUK_INVALID_सूचकांक                 DUK_IDX_MIN

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#घोषणा DUK_VARARGS                       ((duk_int_t) (-1))

/* Number of value stack entries (in addition to actual call arguments)
 * guaranteed to be allocated on entry to a Duktape/C function.
 */
#घोषणा DUK_API_ENTRY_ढेर               64U

/* Value types, used by e.g. duk_get_type() */
#घोषणा DUK_TYPE_MIN                      0U
#घोषणा DUK_TYPE_NONE                     0U    /* no value, e.g. invalid index */
#घोषणा DUK_TYPE_UNDEFINED                1U    /* ECMAScript undefined */
#घोषणा DUK_TYPE_NULL                     2U    /* ECMAScript null */
#घोषणा DUK_TYPE_BOOLEAN                  3U    /* ECMAScript boolean: 0 or 1 */
#घोषणा DUK_TYPE_संख्या                   4U    /* ECMAScript number: double */
#घोषणा DUK_TYPE_STRING                   5U    /* ECMAScript string: CESU-8 / extended UTF-8 encoded */
#घोषणा DUK_TYPE_OBJECT                   6U    /* ECMAScript object: includes objects, arrays, functions, threads */
#घोषणा DUK_TYPE_बफर                   7U    /* fixed or dynamic, garbage collected byte buffer */
#घोषणा DUK_TYPE_POINTER                  8U    /* raw void pointer */
#घोषणा DUK_TYPE_LIGHTFUNC                9U    /* lightweight function pointer */
#घोषणा DUK_TYPE_MAX                      9U

/* Value mask types, used by e.g. duk_get_type_mask() */
#घोषणा DUK_TYPE_MASK_NONE                (1U << DUK_TYPE_NONE)
#घोषणा DUK_TYPE_MASK_UNDEFINED           (1U << DUK_TYPE_UNDEFINED)
#घोषणा DUK_TYPE_MASK_NULL                (1U << DUK_TYPE_NULL)
#घोषणा DUK_TYPE_MASK_BOOLEAN             (1U << DUK_TYPE_BOOLEAN)
#घोषणा DUK_TYPE_MASK_संख्या              (1U << DUK_TYPE_संख्या)
#घोषणा DUK_TYPE_MASK_STRING              (1U << DUK_TYPE_STRING)
#घोषणा DUK_TYPE_MASK_OBJECT              (1U << DUK_TYPE_OBJECT)
#घोषणा DUK_TYPE_MASK_बफर              (1U << DUK_TYPE_बफर)
#घोषणा DUK_TYPE_MASK_POINTER             (1U << DUK_TYPE_POINTER)
#घोषणा DUK_TYPE_MASK_LIGHTFUNC           (1U << DUK_TYPE_LIGHTFUNC)
#घोषणा DUK_TYPE_MASK_THROW               (1U << 10)  /* internal flag value: throw if mask doesn't match */
#घोषणा DUK_TYPE_MASK_PROMOTE             (1U << 11)  /* internal flag value: promote to object if mask matches */

/* Coercion hints */
#घोषणा DUK_HINT_NONE                     0    /* prefer number, unless input is a Date, in which
                                                * case prefer string (E5 Section 8.12.8)
                                                */
#घोषणा DUK_HINT_STRING                   1    /* prefer string */
#घोषणा DUK_HINT_संख्या                   2    /* prefer number */

/* Enumeration flags for duk_enum() */
#घोषणा DUK_ENUM_INCLUDE_NONENUMERABLE    (1U << 0)    /* enumerate non-numerable properties in addition to enumerable */
#घोषणा DUK_ENUM_INCLUDE_HIDDEN           (1U << 1)    /* enumerate hidden symbols too (in Duktape 1.x called internal properties) */
#घोषणा DUK_ENUM_INCLUDE_SYMBOLS          (1U << 2)    /* enumerate symbols */
#घोषणा DUK_ENUM_EXCLUDE_STRINGS          (1U << 3)    /* exclude strings */
#घोषणा DUK_ENUM_OWN_PROPERTIES_ONLY      (1U << 4)    /* don't walk prototype chain, only check own properties */
#घोषणा DUK_ENUM_ARRAY_INDICES_ONLY       (1U << 5)    /* only enumerate array indices */
/* XXX: misleading name */
#घोषणा DUK_ENUM_SORT_ARRAY_INDICES       (1U << 6)    /* sort array indices (applied to full enumeration result, including inherited array indices); XXX: misleading name */
#घोषणा DUK_ENUM_NO_PROXY_BEHAVIOR        (1U << 7)    /* enumerate a proxy object itself without invoking proxy behavior */

/* Compilation flags for duk_compile() and duk_eval() */
/* DUK_COMPILE_xxx bits 0-2 are reserved for an internal 'nargs' argument.
 */
#घोषणा DUK_COMPILE_EVAL                  (1U << 3)    /* compile eval code (instead of global code) */
#घोषणा DUK_COMPILE_FUNCTION              (1U << 4)    /* compile function code (instead of global code) */
#घोषणा DUK_COMPILE_STRICT                (1U << 5)    /* use strict (outer) context for global, eval, or function code */
#घोषणा DUK_COMPILE_SHEBANG               (1U << 6)    /* allow shebang ('#! ...') comment on first line of source */
#घोषणा DUK_COMPILE_SAFE                  (1U << 7)    /* (internal) catch compilation errors */
#घोषणा DUK_COMPILE_NORESULT              (1U << 8)    /* (internal) omit eval result */
#घोषणा DUK_COMPILE_NOSOURCE              (1U << 9)    /* (internal) no source string on stack */
#घोषणा DUK_COMPILE_STRLEN                (1U << 10)   /* (internal) take strlen() of src_buffer (avoids double evaluation in macro) */
#घोषणा DUK_COMPILE_NOFILENAME            (1U << 11)   /* (internal) no filename on stack */
#घोषणा DUK_COMPILE_FUNCEXPR              (1U << 12)   /* (internal) source is a function expression (used for Function constructor) */

/* Flags for duk_def_prop() and its variants; base flags + a lot of convenience shorthands */
#घोषणा DUK_DEFPROP_WRITABLE              (1U << 0)    /* set writable (effective if DUK_DEFPROP_HAVE_WRITABLE set) */
#घोषणा DUK_DEFPROP_ENUMERABLE            (1U << 1)    /* set enumerable (effective if DUK_DEFPROP_HAVE_ENUMERABLE set) */
#घोषणा DUK_DEFPROP_CONFIGURABLE          (1U << 2)    /* set configurable (effective if DUK_DEFPROP_HAVE_CONFIGURABLE set) */
#घोषणा DUK_DEFPROP_HAVE_WRITABLE         (1U << 3)    /* set/clear writable */
#घोषणा DUK_DEFPROP_HAVE_ENUMERABLE       (1U << 4)    /* set/clear enumerable */
#घोषणा DUK_DEFPROP_HAVE_CONFIGURABLE     (1U << 5)    /* set/clear configurable */
#घोषणा DUK_DEFPROP_HAVE_मूल्य            (1U << 6)    /* set value (given on value stack) */
#घोषणा DUK_DEFPROP_HAVE_GETTER           (1U << 7)    /* set getter (given on value stack) */
#घोषणा DUK_DEFPROP_HAVE_SETTER           (1U << 8)    /* set setter (given on value stack) */
#घोषणा DUK_DEFPROP_FORCE                 (1U << 9)    /* force change if possible, may still fail for e.g. virtual properties */
#घोषणा DUK_DEFPROP_SET_WRITABLE          (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)
#घोषणा DUK_DEFPROP_CLEAR_WRITABLE        DUK_DEFPROP_HAVE_WRITABLE
#घोषणा DUK_DEFPROP_SET_ENUMERABLE        (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)
#घोषणा DUK_DEFPROP_CLEAR_ENUMERABLE      DUK_DEFPROP_HAVE_ENUMERABLE
#घोषणा DUK_DEFPROP_SET_CONFIGURABLE      (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)
#घोषणा DUK_DEFPROP_CLEAR_CONFIGURABLE    DUK_DEFPROP_HAVE_CONFIGURABLE
#घोषणा DUK_DEFPROP_W                     DUK_DEFPROP_WRITABLE
#घोषणा DUK_DEFPROP_E                     DUK_DEFPROP_ENUMERABLE
#घोषणा DUK_DEFPROP_C                     DUK_DEFPROP_CONFIGURABLE
#घोषणा DUK_DEFPROP_WE                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE)
#घोषणा DUK_DEFPROP_WC                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_CONFIGURABLE)
#घोषणा DUK_DEFPROP_EC                    (DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#घोषणा DUK_DEFPROP_WEC                   (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#घोषणा DUK_DEFPROP_HAVE_W                DUK_DEFPROP_HAVE_WRITABLE
#घोषणा DUK_DEFPROP_HAVE_E                DUK_DEFPROP_HAVE_ENUMERABLE
#घोषणा DUK_DEFPROP_HAVE_C                DUK_DEFPROP_HAVE_CONFIGURABLE
#घोषणा DUK_DEFPROP_HAVE_WE               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE)
#घोषणा DUK_DEFPROP_HAVE_WC               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#घोषणा DUK_DEFPROP_HAVE_EC               (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#घोषणा DUK_DEFPROP_HAVE_WEC              (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#घोषणा DUK_DEFPROP_SET_W                 DUK_DEFPROP_SET_WRITABLE
#घोषणा DUK_DEFPROP_SET_E                 DUK_DEFPROP_SET_ENUMERABLE
#घोषणा DUK_DEFPROP_SET_C                 DUK_DEFPROP_SET_CONFIGURABLE
#घोषणा DUK_DEFPROP_SET_WE                (DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_ENUMERABLE)
#घोषणा DUK_DEFPROP_SET_WC                (DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE)
#घोषणा DUK_DEFPROP_SET_EC                (DUK_DEFPROP_SET_ENUMERABLE | DUK_DEFPROP_SET_CONFIGURABLE)
#घोषणा DUK_DEFPROP_SET_WEC               (DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_ENUMERABLE | DUK_DEFPROP_SET_CONFIGURABLE)
#घोषणा DUK_DEFPROP_CLEAR_W               DUK_DEFPROP_CLEAR_WRITABLE
#घोषणा DUK_DEFPROP_CLEAR_E               DUK_DEFPROP_CLEAR_ENUMERABLE
#घोषणा DUK_DEFPROP_CLEAR_C               DUK_DEFPROP_CLEAR_CONFIGURABLE
#घोषणा DUK_DEFPROP_CLEAR_WE              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE)
#घोषणा DUK_DEFPROP_CLEAR_WC              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#घोषणा DUK_DEFPROP_CLEAR_EC              (DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#घोषणा DUK_DEFPROP_CLEAR_WEC             (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#घोषणा DUK_DEFPROP_ATTR_W                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W)
#घोषणा DUK_DEFPROP_ATTR_E                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E)
#घोषणा DUK_DEFPROP_ATTR_C                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C)
#घोषणा DUK_DEFPROP_ATTR_WE               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE)
#घोषणा DUK_DEFPROP_ATTR_WC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC)
#घोषणा DUK_DEFPROP_ATTR_EC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_EC)
#घोषणा DUK_DEFPROP_ATTR_WEC              (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC)

/* Flags for duk_push_thread_raw() */
#घोषणा DUK_THREAD_NEW_GLOBAL_ENV         (1U << 0)    /* create a new global environment */

/* Flags for duk_gc() */
#घोषणा DUK_GC_COMPACT                    (1U << 0)    /* compact heap objects */

/* Error codes (must be 8 bits at most, see duk_error.h) */
#घोषणा DUK_ERR_NONE                      0    /* no error (e.g. from duk_get_error_code()) */
#घोषणा DUK_ERR_ERROR                     1    /* Error */
#घोषणा DUK_ERR_EVAL_ERROR                2    /* EvalError */
#घोषणा DUK_ERR_RANGE_ERROR               3    /* RangeError */
#घोषणा DUK_ERR_REFERENCE_ERROR           4    /* ReferenceError */
#घोषणा DUK_ERR_SYNTAX_ERROR              5    /* SyntaxError */
#घोषणा DUK_ERR_TYPE_ERROR                6    /* TypeError */
#घोषणा DUK_ERR_URI_ERROR                 7    /* URIError */

/* Return codes for C functions (shortcut for throwing an error) */
#घोषणा DUK_RET_ERROR                     (-DUK_ERR_ERROR)
#घोषणा DUK_RET_EVAL_ERROR                (-DUK_ERR_EVAL_ERROR)
#घोषणा DUK_RET_RANGE_ERROR               (-DUK_ERR_RANGE_ERROR)
#घोषणा DUK_RET_REFERENCE_ERROR           (-DUK_ERR_REFERENCE_ERROR)
#घोषणा DUK_RET_SYNTAX_ERROR              (-DUK_ERR_SYNTAX_ERROR)
#घोषणा DUK_RET_TYPE_ERROR                (-DUK_ERR_TYPE_ERROR)
#घोषणा DUK_RET_URI_ERROR                 (-DUK_ERR_URI_ERROR)

/* Return codes for protected calls (duk_safe_call(), duk_pcall()) */
#घोषणा DUK_EXEC_SUCCESS                  0
#घोषणा DUK_EXEC_ERROR                    1

/* Debug levels for DUK_USE_DEBUG_WRITE(). */
#घोषणा DUK_LEVEL_DEBUG                   0
#घोषणा DUK_LEVEL_DDEBUG                  1
#घोषणा DUK_LEVEL_DDDEBUG                 2

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

#घोषणा DUK_HIDDEN_प्रतीक(x)     ("\xFF" x)
#घोषणा DUK_GLOBAL_प्रतीक(x)     ("\x80" x)
#घोषणा DUK_LOCAL_प्रतीक(x,uniq) ("\x81" x "\xff" uniq)
#घोषणा DUK_WELLKNOWN_प्रतीक(x)  ("\x81" x "\xff")
#घोषणा DUK_INTERNAL_प्रतीक(x)   ("\x82" x)

/*
 *  If no variadic macros, __FILE__ and __LINE__ are passed through globals
 *  which is ugly and not thread safe.
 */

#अगर !defined(DUK_API_VARIADIC_MACROS)
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_api_global_filename;
DUK_EXTERNAL_DECL duk_int_t duk_api_global_पंक्ति;
#पूर्ण_अगर

/*
 *  Context management
 */

DUK_EXTERNAL_DECL
duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             व्योम *heap_udata,
                             duk_fatal_function fatal_handler);
DUK_EXTERNAL_DECL व्योम duk_destroy_heap(duk_context *ctx);

DUK_EXTERNAL_DECL व्योम duk_suspend(duk_context *ctx, duk_thread_state *state);
DUK_EXTERNAL_DECL व्योम duk_resume(duk_context *ctx, स्थिर duk_thread_state *state);

#घोषणा duk_create_heap_default() \
	duk_create_heap(शून्य, शून्य, शून्य, शून्य, शून्य)

/*
 *  Memory management
 *
 *  Raw functions have no side effects (cannot trigger GC).
 */

DUK_EXTERNAL_DECL व्योम *duk_alloc_raw(duk_context *ctx, duk_आकार_t आकार);
DUK_EXTERNAL_DECL व्योम duk_free_raw(duk_context *ctx, व्योम *ptr);
DUK_EXTERNAL_DECL व्योम *duk_realloc_raw(duk_context *ctx, व्योम *ptr, duk_आकार_t आकार);
DUK_EXTERNAL_DECL व्योम *duk_alloc(duk_context *ctx, duk_आकार_t आकार);
DUK_EXTERNAL_DECL व्योम duk_free(duk_context *ctx, व्योम *ptr);
DUK_EXTERNAL_DECL व्योम *duk_realloc(duk_context *ctx, व्योम *ptr, duk_आकार_t आकार);
DUK_EXTERNAL_DECL व्योम duk_get_memory_functions(duk_context *ctx, duk_memory_functions *out_funcs);
DUK_EXTERNAL_DECL व्योम duk_gc(duk_context *ctx, duk_uint_t flags);

/*
 *  Error handling
 */

DUK_API_NORETURN(DUK_EXTERNAL_DECL व्योम duk_throw_raw(duk_context *ctx));
#घोषणा duk_throw(ctx) \
	(duk_throw_raw((ctx)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL व्योम duk_fatal_raw(duk_context *ctx, स्थिर अक्षर *err_संदेश));
#घोषणा duk_fatal(ctx,err_संदेश) \
	(duk_fatal_raw((ctx), (err_संदेश)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL व्योम duk_error_raw(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *filename, duk_int_t पंक्ति, स्थिर अक्षर *fmt, ...));

#अगर defined(DUK_API_VARIADIC_MACROS)
#घोषणा duk_error(ctx,err_code,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) (err_code), (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_generic_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_eval_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_range_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_reference_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_syntax_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_type_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#घोषणा duk_uri_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#अन्यथा  /* DUK_API_VARIADIC_MACROS */
/* For legacy compilers without variadic macros a macro hack is used to allow
 * variable arguments.  While the macro allows "return duk_error(...)", it
 * will fail with e.g. "(void) duk_error(...)".  The calls are noreturn but
 * with a return value to allow the "return duk_error(...)" idiom.  This may
 * cause some compiler warnings, but without noreturn the generated code is
 * often worse.  The same approach as with variadic macros (using
 * "(duk_error(...), 0)") won't work due to the macro hack structure.
 */
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_error_stash(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_generic_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_eval_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_range_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_reference_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_syntax_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_type_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_uri_error_stash(duk_context *ctx, स्थिर अक्षर *fmt, ...));
#घोषणा duk_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_error_stash)  /* last value is func pointer, arguments follow in parens */
#घोषणा duk_generic_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_generic_error_stash)
#घोषणा duk_eval_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_eval_error_stash)
#घोषणा duk_range_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_range_error_stash)
#घोषणा duk_reference_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_reference_error_stash)
#घोषणा duk_syntax_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_syntax_error_stash)
#घोषणा duk_type_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_type_error_stash)
#घोषणा duk_uri_error  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_uri_error_stash)
#पूर्ण_अगर  /* DUK_API_VARIADIC_MACROS */

DUK_API_NORETURN(DUK_EXTERNAL_DECL व्योम duk_error_va_raw(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *filename, duk_int_t पंक्ति, स्थिर अक्षर *fmt, बहु_सूची ap));

#घोषणा duk_error_va(ctx,err_code,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) (err_code), (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_generic_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_eval_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_range_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_reference_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_syntax_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_type_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#घोषणा duk_uri_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap)), (duk_ret_t) 0)

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
DUK_EXTERNAL_DECL व्योम duk_require_valid_सूचकांक(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_idx_t duk_get_शीर्ष(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_set_शीर्ष(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_get_शीर्ष_सूचकांक(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_शीर्ष_सूचकांक(duk_context *ctx);

/* Although extra/top could be an unsigned type here, using a signed type
 * makes the API more robust to calling code calculation errors or corner
 * cases (where caller might occasionally come up with negative values).
 * Negative values are treated as zero, which is better than casting them
 * to a large unsigned number.  (This principle is used elsewhere in the
 * API too.)
 */
DUK_EXTERNAL_DECL duk_bool_t duk_check_ढेर(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL व्योम duk_require_ढेर(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL duk_bool_t duk_check_ढेर_शीर्ष(duk_context *ctx, duk_idx_t शीर्ष);
DUK_EXTERNAL_DECL व्योम duk_require_ढेर_शीर्ष(duk_context *ctx, duk_idx_t शीर्ष);

/*
 *  Stack manipulation (other than push/pop)
 */

DUK_EXTERNAL_DECL व्योम duk_swap(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL व्योम duk_swap_शीर्ष(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_dup(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL व्योम duk_dup_शीर्ष(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_जोड़ो(duk_context *ctx, duk_idx_t to_idx);
DUK_EXTERNAL_DECL व्योम duk_pull(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL व्योम duk_replace(duk_context *ctx, duk_idx_t to_idx);
DUK_EXTERNAL_DECL व्योम duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t to_idx);
DUK_EXTERNAL_DECL व्योम duk_remove(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_xcopymove_raw(duk_context *to_ctx, duk_context *from_ctx, duk_idx_t गिनती, duk_bool_t is_copy);

#घोषणा duk_xmove_शीर्ष(to_ctx,from_ctx,गिनती) \
	duk_xcopymove_raw((to_ctx), (from_ctx), (गिनती), 0 /*is_copy*/)
#घोषणा duk_xcopy_शीर्ष(to_ctx,from_ctx,गिनती) \
	duk_xcopymove_raw((to_ctx), (from_ctx), (गिनती), 1 /*is_copy*/)

/*
 *  Push operations
 *
 *  Push functions return the absolute (relative to bottom of frame)
 *  position of the pushed value for convenience.
 *
 *  Note: duk_dup() is technically a push.
 */

DUK_EXTERNAL_DECL व्योम duk_धकेलो_undefined(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_null(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_boolean(duk_context *ctx, duk_bool_t val);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_true(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_false(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_संख्या(duk_context *ctx, duk_double_t val);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_nan(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_int(duk_context *ctx, duk_int_t val);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_uint(duk_context *ctx, duk_uint_t val);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_धकेलो_string(duk_context *ctx, स्थिर अक्षर *str);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_धकेलो_lstring(duk_context *ctx, स्थिर अक्षर *str, duk_आकार_t len);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_pointer(duk_context *ctx, व्योम *p);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_धकेलो_sprintf(duk_context *ctx, स्थिर अक्षर *fmt, ...);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_धकेलो_vsprintf(duk_context *ctx, स्थिर अक्षर *fmt, बहु_सूची ap);

/* duk_push_literal() may evaluate its argument (a C string literal) more than
 * once on purpose.  When speed is preferred, sizeof() avoids an unnecessary
 * strlen() at runtime.  Sizeof("foo") == 4, so subtract 1.  The argument
 * must be non-NULL and should not contain internal NUL characters as the
 * behavior will then depend on config options.
 */
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_धकेलो_literal(ctx,cstring)  duk_धकेलो_string((ctx), (cstring))
#अन्यथा
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_धकेलो_literal_raw(duk_context *ctx, स्थिर अक्षर *str, duk_आकार_t len);
#घोषणा duk_धकेलो_literal(ctx,cstring)  duk_धकेलो_literal_raw((ctx), (cstring), माप((cstring)) - 1U)
#पूर्ण_अगर

DUK_EXTERNAL_DECL व्योम duk_धकेलो_this(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_new_target(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_current_function(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_current_thread(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_global_object(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_heap_stash(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_global_stash(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_धकेलो_thread_stash(duk_context *ctx, duk_context *target_ctx);

DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_bare_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_bare_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_c_lightfunc(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_idx_t लंबाई, duk_int_t magic);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_thread_raw(duk_context *ctx, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_proxy(duk_context *ctx, duk_uint_t proxy_flags);

#घोषणा duk_धकेलो_thread(ctx) \
	duk_धकेलो_thread_raw((ctx), 0 /*flags*/)

#घोषणा duk_धकेलो_thread_new_globalenv(ctx) \
	duk_धकेलो_thread_raw((ctx), DUK_THREAD_NEW_GLOBAL_ENV /*flags*/)

DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_error_object_raw(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *filename, duk_int_t पंक्ति, स्थिर अक्षर *fmt, ...);

#अगर defined(DUK_API_VARIADIC_MACROS)
#घोषणा duk_धकेलो_error_object(ctx,err_code,...)  \
	duk_धकेलो_error_object_raw((ctx), (err_code), (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), __VA_ARGS__)
#अन्यथा
DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_error_object_stash(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *fmt, ...);
/* Note: parentheses are required so that the comma expression works in assignments. */
#घोषणा duk_धकेलो_error_object  \
	(duk_api_global_filename = (स्थिर अक्षर *) (DUK_FILE_MACRO), \
	 duk_api_global_पंक्ति = (duk_int_t) (DUK_पंक्ति_MACRO), \
	 duk_धकेलो_error_object_stash)  /* last value is func pointer, arguments follow in parens */
#पूर्ण_अगर

DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_error_object_va_raw(duk_context *ctx, duk_errcode_t err_code, स्थिर अक्षर *filename, duk_int_t पंक्ति, स्थिर अक्षर *fmt, बहु_सूची ap);
#घोषणा duk_धकेलो_error_object_va(ctx,err_code,fmt,ap)  \
	duk_धकेलो_error_object_va_raw((ctx), (err_code), (स्थिर अक्षर *) (DUK_FILE_MACRO), (duk_int_t) (DUK_पंक्ति_MACRO), (fmt), (ap))

#घोषणा DUK_BUF_FLAG_DYNAMIC   (1 << 0)    /* internal flag: dynamic buffer */
#घोषणा DUK_BUF_FLAG_EXTERNAL  (1 << 1)    /* internal flag: external buffer */
#घोषणा DUK_BUF_FLAG_NOZERO    (1 << 2)    /* internal flag: don't zero allocated buffer */

DUK_EXTERNAL_DECL व्योम *duk_धकेलो_बफर_raw(duk_context *ctx, duk_आकार_t आकार, duk_small_uint_t flags);

#घोषणा duk_धकेलो_बफर(ctx,आकार,dynamic) \
	duk_धकेलो_बफर_raw((ctx), (आकार), (dynamic) ? DUK_BUF_FLAG_DYNAMIC : 0)
#घोषणा duk_धकेलो_fixed_बफर(ctx,आकार) \
	duk_धकेलो_बफर_raw((ctx), (आकार), 0 /*flags*/)
#घोषणा duk_धकेलो_dynamic_बफर(ctx,आकार) \
	duk_धकेलो_बफर_raw((ctx), (आकार), DUK_BUF_FLAG_DYNAMIC /*flags*/)
#घोषणा duk_धकेलो_external_बफर(ctx) \
	((व्योम) duk_धकेलो_बफर_raw((ctx), 0, DUK_BUF_FLAG_DYNAMIC | DUK_BUF_FLAG_EXTERNAL))

#घोषणा DUK_BUFOBJ_ARRAYBUFFER         0
#घोषणा DUK_BUFOBJ_NODEJS_बफर       1
#घोषणा DUK_BUFOBJ_DATAVIEW            2
#घोषणा DUK_BUFOBJ_INT8ARRAY           3
#घोषणा DUK_BUFOBJ_UINT8ARRAY          4
#घोषणा DUK_BUFOBJ_UINT8CLAMPEDARRAY   5
#घोषणा DUK_BUFOBJ_INT16ARRAY          6
#घोषणा DUK_BUFOBJ_UINT16ARRAY         7
#घोषणा DUK_BUFOBJ_INT32ARRAY          8
#घोषणा DUK_BUFOBJ_UINT32ARRAY         9
#घोषणा DUK_BUFOBJ_FLOAT32ARRAY        10
#घोषणा DUK_BUFOBJ_FLOAT64ARRAY        11

DUK_EXTERNAL_DECL व्योम duk_धकेलो_बफर_object(duk_context *ctx, duk_idx_t idx_बफर, duk_आकार_t byte_offset, duk_आकार_t byte_लंबाई, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_idx_t duk_धकेलो_heapptr(duk_context *ctx, व्योम *ptr);

/*
 *  Pop operations
 */

DUK_EXTERNAL_DECL व्योम duk_निकालो(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_निकालो_n(duk_context *ctx, duk_idx_t गिनती);
DUK_EXTERNAL_DECL व्योम duk_निकालो_2(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_निकालो_3(duk_context *ctx);

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t idx, duk_int_t type);
DUK_EXTERNAL_DECL duk_uint_t duk_get_type_mask(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t mask);

DUK_EXTERNAL_DECL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx);
#घोषणा duk_is_null_or_undefined(ctx, idx) \
	((duk_get_type_mask((ctx), (idx)) & (DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_UNDEFINED)) ? 1 : 0)

DUK_EXTERNAL_DECL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_संख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_बफर_तथ्य(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_प्रतीक(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx);

#घोषणा duk_is_callable(ctx,idx) \
	duk_is_function((ctx), (idx))
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructable(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_dynamic_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_fixed_बफर(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_external_बफर(duk_context *ctx, duk_idx_t idx);

/* Buffers and lightfuncs are not considered primitive because they mimic
 * objects and e.g. duk_to_primitive() will coerce them instead of returning
 * them as is.  Symbols are represented as strings internally.
 */
#घोषणा duk_is_primitive(ctx,idx) \
	duk_check_type_mask((ctx), (idx), DUK_TYPE_MASK_UNDEFINED | \
	                                  DUK_TYPE_MASK_NULL | \
	                                  DUK_TYPE_MASK_BOOLEAN | \
	                                  DUK_TYPE_MASK_संख्या | \
	                                  DUK_TYPE_MASK_STRING | \
	                                  DUK_TYPE_MASK_POINTER)

/* Symbols are object coercible, covered by DUK_TYPE_MASK_STRING. */
#घोषणा duk_is_object_coercible(ctx,idx) \
	duk_check_type_mask((ctx), (idx), DUK_TYPE_MASK_BOOLEAN | \
	                                  DUK_TYPE_MASK_संख्या | \
	                                  DUK_TYPE_MASK_STRING | \
	                                  DUK_TYPE_MASK_OBJECT | \
	                                  DUK_TYPE_MASK_बफर | \
	                                  DUK_TYPE_MASK_POINTER | \
	                                  DUK_TYPE_MASK_LIGHTFUNC)

DUK_EXTERNAL_DECL duk_errcode_t duk_get_error_code(duk_context *ctx, duk_idx_t idx);
#घोषणा duk_is_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) != 0)
#घोषणा duk_is_eval_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_EVAL_ERROR)
#घोषणा duk_is_range_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_RANGE_ERROR)
#घोषणा duk_is_reference_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_REFERENCE_ERROR)
#घोषणा duk_is_syntax_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_SYNTAX_ERROR)
#घोषणा duk_is_type_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_TYPE_ERROR)
#घोषणा duk_is_uri_error(ctx,idx) \
	(duk_get_error_code((ctx), (idx)) == DUK_ERR_URI_ERROR)

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_get_संख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_get_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len);
DUK_EXTERNAL_DECL व्योम *duk_get_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार);
DUK_EXTERNAL_DECL व्योम *duk_get_बफर_तथ्य(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार);
DUK_EXTERNAL_DECL व्योम *duk_get_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_get_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम *duk_get_heapptr(duk_context *ctx, duk_idx_t idx);

/*
 *  Get-with-explicit default operations: like get operations but with an
 *  explicit default value.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean_default(duk_context *ctx, duk_idx_t idx, duk_bool_t def_मूल्य);
DUK_EXTERNAL_DECL duk_double_t duk_get_संख्या_default(duk_context *ctx, duk_idx_t idx, duk_double_t def_मूल्य);
DUK_EXTERNAL_DECL duk_int_t duk_get_int_default(duk_context *ctx, duk_idx_t idx, duk_int_t def_मूल्य);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint_default(duk_context *ctx, duk_idx_t idx, duk_uint_t def_मूल्य);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_get_string_default(duk_context *ctx, duk_idx_t idx, स्थिर अक्षर *def_मूल्य);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_get_lstring_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len, स्थिर अक्षर *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL व्योम *duk_get_बफर_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार, व्योम *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL व्योम *duk_get_बफर_तथ्य_default(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार, व्योम *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL व्योम *duk_get_pointer_default(duk_context *ctx, duk_idx_t idx, व्योम *def_मूल्य);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function_default(duk_context *ctx, duk_idx_t idx, duk_c_function def_मूल्य);
DUK_EXTERNAL_DECL duk_context *duk_get_context_default(duk_context *ctx, duk_idx_t idx, duk_context *def_मूल्य);
DUK_EXTERNAL_DECL व्योम *duk_get_heapptr_default(duk_context *ctx, duk_idx_t idx, व्योम *def_मूल्य);

/*
 *  Opt operations: like require operations but with an explicit default value
 *  when value is undefined or index is invalid, null and non-matching types
 *  cause a TypeError.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_opt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_मूल्य);
DUK_EXTERNAL_DECL duk_double_t duk_opt_संख्या(duk_context *ctx, duk_idx_t idx, duk_double_t def_मूल्य);
DUK_EXTERNAL_DECL duk_int_t duk_opt_int(duk_context *ctx, duk_idx_t idx, duk_int_t def_मूल्य);
DUK_EXTERNAL_DECL duk_uint_t duk_opt_uint(duk_context *ctx, duk_idx_t idx, duk_uint_t def_मूल्य);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_opt_string(duk_context *ctx, duk_idx_t idx, स्थिर अक्षर *def_ptr);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_opt_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len, स्थिर अक्षर *def_ptr, duk_आकार_t def_len);
DUK_EXTERNAL_DECL व्योम *duk_opt_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार, व्योम *def_ptr, duk_आकार_t def_आकार);
DUK_EXTERNAL_DECL व्योम *duk_opt_बफर_तथ्य(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार, व्योम *def_ptr, duk_आकार_t def_आकार);
DUK_EXTERNAL_DECL व्योम *duk_opt_pointer(duk_context *ctx, duk_idx_t idx, व्योम *def_मूल्य);
DUK_EXTERNAL_DECL duk_c_function duk_opt_c_function(duk_context *ctx, duk_idx_t idx, duk_c_function def_मूल्य);
DUK_EXTERNAL_DECL duk_context *duk_opt_context(duk_context *ctx, duk_idx_t idx, duk_context *def_मूल्य);
DUK_EXTERNAL_DECL व्योम *duk_opt_heapptr(duk_context *ctx, duk_idx_t idx, व्योम *def_मूल्य);

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

#घोषणा duk_require_type_mask(ctx,idx,mask) \
	((व्योम) duk_check_type_mask((ctx), (idx), (mask) | DUK_TYPE_MASK_THROW))

DUK_EXTERNAL_DECL व्योम duk_require_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_require_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_require_संख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_require_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len);
DUK_EXTERNAL_DECL व्योम duk_require_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम *duk_require_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार);
DUK_EXTERNAL_DECL व्योम *duk_require_बफर_तथ्य(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार);
DUK_EXTERNAL_DECL व्योम *duk_require_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_require_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_require_function(duk_context *ctx, duk_idx_t idx);
#घोषणा duk_require_callable(ctx,idx) \
	duk_require_function((ctx), (idx))
DUK_EXTERNAL_DECL व्योम duk_require_constructor_call(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_require_constructable(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम *duk_require_heapptr(duk_context *ctx, duk_idx_t idx);

/* Symbols are object coercible and covered by DUK_TYPE_MASK_STRING. */
#घोषणा duk_require_object_coercible(ctx,idx) \
	((व्योम) duk_check_type_mask((ctx), (idx), DUK_TYPE_MASK_BOOLEAN | \
	                                            DUK_TYPE_MASK_संख्या | \
	                                            DUK_TYPE_MASK_STRING | \
	                                            DUK_TYPE_MASK_OBJECT | \
	                                            DUK_TYPE_MASK_बफर | \
	                                            DUK_TYPE_MASK_POINTER | \
	                                            DUK_TYPE_MASK_LIGHTFUNC | \
	                                            DUK_TYPE_MASK_THROW))

/*
 *  Coercion operations: in-place coercion, return coerced value where
 *  applicable.  If index is invalid, throw error.  Some coercions may
 *  throw an expected error (e.g. from a toString() or valueOf() call)
 *  or an internal error (e.g. from out of memory).
 */

DUK_EXTERNAL_DECL व्योम duk_to_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_to_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_to_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_to_संख्या(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_to_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_to_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int32_t duk_to_int32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint32_t duk_to_uint32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint16_t duk_to_uint16(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_to_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_to_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len);
DUK_EXTERNAL_DECL व्योम *duk_to_बफर_raw(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार, duk_uint_t flags);
DUK_EXTERNAL_DECL व्योम *duk_to_pointer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_to_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_to_primitive(duk_context *ctx, duk_idx_t idx, duk_int_t hint);

#घोषणा DUK_BUF_MODE_FIXED      0   /* internal: request fixed buffer result */
#घोषणा DUK_BUF_MODE_DYNAMIC    1   /* internal: request dynamic buffer result */
#घोषणा DUK_BUF_MODE_DONTCARE   2   /* internal: don't care about fixed/dynamic nature */

#घोषणा duk_to_बफर(ctx,idx,out_आकार) \
	duk_to_बफर_raw((ctx), (idx), (out_आकार), DUK_BUF_MODE_DONTCARE)
#घोषणा duk_to_fixed_बफर(ctx,idx,out_आकार) \
	duk_to_बफर_raw((ctx), (idx), (out_आकार), DUK_BUF_MODE_FIXED)
#घोषणा duk_to_dynamic_बफर(ctx,idx,out_आकार) \
	duk_to_बफर_raw((ctx), (idx), (out_आकार), DUK_BUF_MODE_DYNAMIC)

/* safe variants of a few coercion operations */
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_safe_to_lstring(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_len);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_to_stacktrace(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_safe_to_stacktrace(duk_context *ctx, duk_idx_t idx);
#घोषणा duk_safe_to_string(ctx,idx) \
	duk_safe_to_lstring((ctx), (idx), शून्य)

/*
 *  Value length
 */

DUK_EXTERNAL_DECL duk_आकार_t duk_get_लंबाई(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_set_लंबाई(duk_context *ctx, duk_idx_t idx, duk_आकार_t len);
#अगर 0
/* duk_require_length()? */
/* duk_opt_length()? */
#पूर्ण_अगर

/*
 *  Misc conversion
 */

DUK_EXTERNAL_DECL स्थिर अक्षर *duk_base64_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_base64_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_hex_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_hex_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL स्थिर अक्षर *duk_json_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_json_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags);
DUK_EXTERNAL_DECL व्योम duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags);

DUK_EXTERNAL_DECL स्थिर अक्षर *duk_बफर_to_string(duk_context *ctx, duk_idx_t idx);

/*
 *  Buffer
 */

DUK_EXTERNAL_DECL व्योम *duk_resize_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t new_आकार);
DUK_EXTERNAL_DECL व्योम *duk_steal_बफर(duk_context *ctx, duk_idx_t idx, duk_आकार_t *out_आकार);
DUK_EXTERNAL_DECL व्योम duk_config_बफर(duk_context *ctx, duk_idx_t idx, व्योम *ptr, duk_आकार_t len);

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
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_get_prop_literal(ctx,obj_idx,कुंजी)  duk_get_prop_string((ctx), (obj_idx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_get_prop_literal(ctx,obj_idx,कुंजी)  duk_get_prop_literal_raw((ctx), (obj_idx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, व्योम *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_put_prop_literal(ctx,obj_idx,कुंजी)  duk_put_prop_string((ctx), (obj_idx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_put_prop_literal(ctx,obj_idx,कुंजी)  duk_put_prop_literal_raw((ctx), (obj_idx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, व्योम *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_del_prop_literal(ctx,obj_idx,कुंजी)  duk_del_prop_string((ctx), (obj_idx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_del_prop_literal(ctx,obj_idx,कुंजी)  duk_del_prop_literal_raw((ctx), (obj_idx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, व्योम *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_has_prop_literal(ctx,obj_idx,कुंजी)  duk_has_prop_string((ctx), (obj_idx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_has_prop_literal(ctx,obj_idx,कुंजी)  duk_has_prop_literal_raw((ctx), (obj_idx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_सूचकांक(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, व्योम *ptr);

DUK_EXTERNAL_DECL व्योम duk_get_prop_desc(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);
DUK_EXTERNAL_DECL व्योम duk_def_prop(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_bool_t duk_get_global_string(duk_context *ctx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_lstring(duk_context *ctx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_get_global_literal(ctx,कुंजी)  duk_get_global_string((ctx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_literal_raw(duk_context *ctx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_get_global_literal(ctx,कुंजी)  duk_get_global_literal_raw((ctx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_heapptr(duk_context *ctx, व्योम *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_string(duk_context *ctx, स्थिर अक्षर *कुंजी);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_lstring(duk_context *ctx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#अगर defined(DUK_USE_PREFER_आकार)
#घोषणा duk_put_global_literal(ctx,कुंजी)  duk_put_global_string((ctx), (कुंजी))
#अन्यथा
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_literal_raw(duk_context *ctx, स्थिर अक्षर *कुंजी, duk_आकार_t कुंजी_len);
#घोषणा duk_put_global_literal(ctx,कुंजी)  duk_put_global_literal_raw((ctx), (कुंजी), माप((कुंजी)) - 1U)
#पूर्ण_अगर
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_heapptr(duk_context *ctx, व्योम *ptr);

/*
 *  Inspection
 */

DUK_EXTERNAL_DECL व्योम duk_inspect_मूल्य(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_inspect_callstack_entry(duk_context *ctx, duk_int_t level);

/*
 *  Object prototype
 */

DUK_EXTERNAL_DECL व्योम duk_get_prototype(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_set_prototype(duk_context *ctx, duk_idx_t idx);

/*
 *  Object finalizer
 */

DUK_EXTERNAL_DECL व्योम duk_get_finalizer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_set_finalizer(duk_context *ctx, duk_idx_t idx);

/*
 *  Global object
 */

DUK_EXTERNAL_DECL व्योम duk_set_global_object(duk_context *ctx);

/*
 *  Duktape/C function magic value
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_magic(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL व्योम duk_set_magic(duk_context *ctx, duk_idx_t idx, duk_int_t magic);
DUK_EXTERNAL_DECL duk_int_t duk_get_current_magic(duk_context *ctx);

/*
 *  Module helpers: put multiple function or constant properties
 */

DUK_EXTERNAL_DECL व्योम duk_put_function_list(duk_context *ctx, duk_idx_t obj_idx, स्थिर duk_function_list_entry *funcs);
DUK_EXTERNAL_DECL व्योम duk_put_संख्या_list(duk_context *ctx, duk_idx_t obj_idx, स्थिर duk_संख्या_list_entry *numbers);

/*
 *  Object operations
 */

DUK_EXTERNAL_DECL व्योम duk_compact(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL व्योम duk_enum(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t enum_flags);
DUK_EXTERNAL_DECL duk_bool_t duk_अग्र(duk_context *ctx, duk_idx_t enum_idx, duk_bool_t get_मूल्य);
DUK_EXTERNAL_DECL व्योम duk_seal(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL व्योम duk_freeze(duk_context *ctx, duk_idx_t obj_idx);

/*
 *  String manipulation
 */

DUK_EXTERNAL_DECL व्योम duk_concat(duk_context *ctx, duk_idx_t गिनती);
DUK_EXTERNAL_DECL व्योम duk_join(duk_context *ctx, duk_idx_t गिनती);
DUK_EXTERNAL_DECL व्योम duk_decode_string(duk_context *ctx, duk_idx_t idx, duk_decode_char_function callback, व्योम *udata);
DUK_EXTERNAL_DECL व्योम duk_map_string(duk_context *ctx, duk_idx_t idx, duk_map_char_function callback, व्योम *udata);
DUK_EXTERNAL_DECL व्योम duk_substring(duk_context *ctx, duk_idx_t idx, duk_आकार_t start_char_offset, duk_आकार_t end_char_offset);
DUK_EXTERNAL_DECL व्योम duk_trim(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_codepoint_t duk_char_code_at(duk_context *ctx, duk_idx_t idx, duk_आकार_t char_offset);

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

DUK_EXTERNAL_DECL व्योम duk_call(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL व्योम duk_call_method(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL व्योम duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_method(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL व्योम duk_new(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_safe_call(duk_context *ctx, duk_safe_call_function func, व्योम *udata, duk_idx_t nargs, duk_idx_t nrets);

/*
 *  Thread management
 */

/* There are currently no native functions to yield/resume, due to the internal
 * limitations on coroutine handling.  These will be added later.
 */

/*
 *  Compilation and evaluation
 */

DUK_EXTERNAL_DECL duk_int_t duk_eval_raw(duk_context *ctx, स्थिर अक्षर *src_बफर, duk_आकार_t src_लंबाई, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_int_t duk_compile_raw(duk_context *ctx, स्थिर अक्षर *src_बफर, duk_आकार_t src_लंबाई, duk_uint_t flags);

/* plain */
#घोषणा duk_eval(ctx)  \
	((व्योम) duk_eval_raw((ctx), शून्य, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOFILENAME))

#घोषणा duk_eval_noresult(ctx)  \
	((व्योम) duk_eval_raw((ctx), शून्य, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval(ctx)  \
	(duk_eval_raw((ctx), शून्य, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval_noresult(ctx)  \
	(duk_eval_raw((ctx), शून्य, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_compile(ctx,flags)  \
	((व्योम) duk_compile_raw((ctx), शून्य, 0, 2 /*args*/ | (flags)))

#घोषणा duk_pcompile(ctx,flags)  \
	(duk_compile_raw((ctx), शून्य, 0, 2 /*args*/ | (flags) | DUK_COMPILE_SAFE))

/* string */
#घोषणा duk_eval_string(ctx,src)  \
	((व्योम) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#घोषणा duk_eval_string_noresult(ctx,src)  \
	((व्योम) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval_string(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval_string_noresult(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_compile_string(ctx,flags,src)  \
	((व्योम) duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#घोषणा duk_compile_string_filename(ctx,flags,src)  \
	((व्योम) duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

#घोषणा duk_pcompile_string(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#घोषणा duk_pcompile_string_filename(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

/* lstring */
#घोषणा duk_eval_lstring(ctx,buf,len)  \
	((व्योम) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#घोषणा duk_eval_lstring_noresult(ctx,buf,len)  \
	((व्योम) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval_lstring(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_SAFE | DUK_COMPILE_NOFILENAME))

#घोषणा duk_peval_lstring_noresult(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#घोषणा duk_compile_lstring(ctx,flags,buf,len)  \
	((व्योम) duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#घोषणा duk_compile_lstring_filename(ctx,flags,buf,len)  \
	((व्योम) duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE))

#घोषणा duk_pcompile_lstring(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#घोषणा duk_pcompile_lstring_filename(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_SAFE | DUK_COMPILE_NOSOURCE))

/*
 *  Bytecode load/dump
 */

DUK_EXTERNAL_DECL व्योम duk_dump_function(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_load_function(duk_context *ctx);

/*
 *  Debugging
 */

DUK_EXTERNAL_DECL व्योम duk_धकेलो_context_dump(duk_context *ctx);

/*
 *  Debugger (debug protocol)
 */

DUK_EXTERNAL_DECL व्योम duk_debugger_attach(duk_context *ctx,
                                           duk_debug_read_function read_cb,
                                           duk_debug_write_function write_cb,
                                           duk_debug_peek_function peek_cb,
                                           duk_debug_read_flush_function read_flush_cb,
                                           duk_debug_write_flush_function write_flush_cb,
                                           duk_debug_request_function request_cb,
                                           duk_debug_detached_function detached_cb,
                                           व्योम *udata);
DUK_EXTERNAL_DECL व्योम duk_debugger_detach(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_debugger_cooperate(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues);
DUK_EXTERNAL_DECL व्योम duk_debugger_pause(duk_context *ctx);

/*
 *  Time handling
 */

DUK_EXTERNAL_DECL duk_double_t duk_get_now(duk_context *ctx);
DUK_EXTERNAL_DECL व्योम duk_time_to_components(duk_context *ctx, duk_double_t timeval, duk_time_components *comp);
DUK_EXTERNAL_DECL duk_double_t duk_components_to_time(duk_context *ctx, duk_time_components *comp);

/*
 *  Date provider related constants
 *
 *  NOTE: These are "semi public" - you should only use these if you write
 *  your own platform specific Date provider, see doc/datetime.rst.
 */

/* Millisecond count constants. */
#घोषणा DUK_DATE_MSEC_SECOND          1000L
#घोषणा DUK_DATE_MSEC_MINUTE          (60L * 1000L)
#घोषणा DUK_DATE_MSEC_HOUR            (60L * 60L * 1000L)
#घोषणा DUK_DATE_MSEC_DAY             (24L * 60L * 60L * 1000L)

/* ECMAScript date range is 100 million days from Epoch:
 * > 100e6 * 24 * 60 * 60 * 1000  // 100M days in millisecs
 * 8640000000000000
 * (= 8.64e15)
 */
#घोषणा DUK_DATE_MSEC_100M_DAYS         (8.64e15)
#घोषणा DUK_DATE_MSEC_100M_DAYS_LEEWAY  (8.64e15 + 24 * 3600e3)

/* ECMAScript year range:
 * > new Date(100e6 * 24 * 3600e3).toISOString()
 * '+275760-09-13T00:00:00.000Z'
 * > new Date(-100e6 * 24 * 3600e3).toISOString()
 * '-271821-04-20T00:00:00.000Z'
 */
#घोषणा DUK_DATE_MIN_ECMA_YEAR     (-271821L)
#घोषणा DUK_DATE_MAX_ECMA_YEAR     275760L

/* Part indices for internal breakdowns.  Part order from DUK_DATE_IDX_YEAR
 * to DUK_DATE_IDX_MILLISECOND matches argument ordering of ECMAScript API
 * calls (like Date constructor call).  Some functions in duk_bi_date.c
 * depend on the specific ordering, so change with care.  16 bits are not
 * enough for all parts (year, specifically).
 *
 * Must be in-sync with genbuiltins.py.
 */
#घोषणा DUK_DATE_IDX_YEAR           0  /* year */
#घोषणा DUK_DATE_IDX_MONTH          1  /* month: 0 to 11 */
#घोषणा DUK_DATE_IDX_DAY            2  /* day within month: 0 to 30 */
#घोषणा DUK_DATE_IDX_HOUR           3
#घोषणा DUK_DATE_IDX_MINUTE         4
#घोषणा DUK_DATE_IDX_SECOND         5
#घोषणा DUK_DATE_IDX_MILLISECOND    6
#घोषणा DUK_DATE_IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#घोषणा DUK_DATE_IDX_अंक_PARTS      8

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

#घोषणा DUK_DATE_FLAG_NAN_TO_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#घोषणा DUK_DATE_FLAG_NAN_TO_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#घोषणा DUK_DATE_FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#घोषणा DUK_DATE_FLAG_EQUIVYEAR            (1 << 3)  /* timeval breakdown: replace year with equivalent year in the [1971,2037] range for DST calculations */
#घोषणा DUK_DATE_FLAG_LOCALTIME            (1 << 4)  /* convert time value to local time */
#घोषणा DUK_DATE_FLAG_SUB1900              (1 << 5)  /* getter: subtract 1900 from year when getting year part */
#घोषणा DUK_DATE_FLAG_TOSTRING_DATE        (1 << 6)  /* include date part in string conversion result */
#घोषणा DUK_DATE_FLAG_TOSTRING_TIME        (1 << 7)  /* include time part in string conversion result */
#घोषणा DUK_DATE_FLAG_TOSTRING_LOCALE      (1 << 8)  /* use locale specific formatting if available */
#घोषणा DUK_DATE_FLAG_TIMESETTER           (1 << 9)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#घोषणा DUK_DATE_FLAG_YEAR_FIXUP           (1 << 10) /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#घोषणा DUK_DATE_FLAG_SEP_T                (1 << 11) /* string conversion: use 'T' instead of ' ' as a separator */
#घोषणा DUK_DATE_FLAG_मूल्य_SHIFT          12        /* additional values begin at bit 12 */

/*
 *  ROM pointer compression
 */

/* Support array for ROM pointer compression.  Only declared when ROM
 * pointer compression is active.
 */
#अगर defined(DUK_USE_ROM_OBJECTS) && defined(DUK_USE_HEAPPTR16)
DUK_EXTERNAL_DECL स्थिर व्योम * स्थिर duk_rom_compressed_pointers[];
#पूर्ण_अगर

/*
 *  C++ name mangling
 */

#अगर defined(__cplusplus)
/* end 'extern "C"' wrapper */
}
#पूर्ण_अगर

/*
 *  END PUBLIC API
 */

#पूर्ण_अगर  /* DUKTAPE_H_INCLUDED */
