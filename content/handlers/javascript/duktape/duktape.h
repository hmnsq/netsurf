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

#ఒకవేళ !defined(DUKTAPE_H_INCLUDED)
#నిర్వచించు DUKTAPE_H_INCLUDED

#నిర్వచించు DUK_SINGLE_FILE

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
#నిర్వచించు DUK_సంస్కరణ                       20700L

/* Git commit, describe, and branch for Duktape build.  Useful for
 * non-official snapshot builds so that application code can easily log
 * which Duktape snapshot was used.  Not available in the ECMAScript
 * environment.
 */
#నిర్వచించు DUK_GIT_COMMIT                    "external"
#నిర్వచించు DUK_GIT_DESCRIBE                  "external"
#నిర్వచించు DUK_GIT_BRANCH                    "external"

/* External duk_config.h provides platform/compiler/OS dependent
 * typedefs and macros, and DUK_USE_xxx config options so that
 * the rest of Duktape doesn't need to do any feature detection.
 * DUK_VERSION is defined before including so that configuration
 * snippets can react to it.
 */
#అన్తర్భావ "duk_config.h"

/*
 *  Avoid C++ name mangling
 */

#ఒకవేళ defined(__cplusplus)
బాహ్య "C" {
#అయితే_ముగింపు

/*
 *  Some defines forwarded from feature detection
 */

#నిర్వచనం_రద్దు DUK_API_VARIADIC_MACROS
#ఒకవేళ defined(DUK_USE_VARIADIC_MACROS)
#నిర్వచించు DUK_API_VARIADIC_MACROS
#అయితే_ముగింపు

#నిర్వచించు DUK_API_NORETURN(decl) DUK_NORETURN(decl)

/*
 *  Public API specific typedefs
 *
 *  Many types are wrapped by Duktape for portability to rare platforms
 *  where e.g. 'int' is a 16-bit type.  See practical typing discussion
 *  in Duktape web documentation.
 */

నిర్మాణం duk_thread_state;
నిర్మాణం duk_జ్ఞాపకం_functions;
నిర్మాణం duk_function_list_నమోదు;
నిర్మాణం duk_సంఖ్య_list_నమోదు;
నిర్మాణం duk_సమయం_components;

/* duk_context is now defined in duk_config.h because it may also be
 * referenced there by prototypes.
 */
రకనిర్వచనం నిర్మాణం duk_thread_state duk_thread_state;
రకనిర్వచనం నిర్మాణం duk_జ్ఞాపకం_functions duk_జ్ఞాపకం_functions;
రకనిర్వచనం నిర్మాణం duk_function_list_నమోదు duk_function_list_నమోదు;
రకనిర్వచనం నిర్మాణం duk_సంఖ్య_list_నమోదు duk_సంఖ్య_list_నమోదు;
రకనిర్వచనం నిర్మాణం duk_సమయం_components duk_సమయం_components;

రకనిర్వచనం duk_ret_t (*duk_c_function)(duk_context *ctx);
రకనిర్వచనం శూన్య *(*duk_alloc_function) (శూన్య *udata, duk_కొలత_t కొలత);
రకనిర్వచనం శూన్య *(*duk_realloc_function) (శూన్య *udata, శూన్య *ptr, duk_కొలత_t కొలత);
రకనిర్వచనం శూన్య (*duk_free_function) (శూన్య *udata, శూన్య *ptr);
రకనిర్వచనం శూన్య (*duk_fatal_function) (శూన్య *udata, స్థిరరాశి అక్షరం *సందేశం);
రకనిర్వచనం శూన్య (*duk_decode_char_function) (శూన్య *udata, duk_codepoint_t codepoint);
రకనిర్వచనం duk_codepoint_t (*duk_map_char_function) (శూన్య *udata, duk_codepoint_t codepoint);
రకనిర్వచనం duk_ret_t (*duk_సురక్షిత_call_function) (duk_context *ctx, శూన్య *udata);
రకనిర్వచనం duk_కొలత_t (*duk_debug_read_function) (శూన్య *udata, అక్షరం *బఫరు, duk_కొలత_t నిడివి);
రకనిర్వచనం duk_కొలత_t (*duk_debug_write_function) (శూన్య *udata, స్థిరరాశి అక్షరం *బఫరు, duk_కొలత_t నిడివి);
రకనిర్వచనం duk_కొలత_t (*duk_debug_peek_function) (శూన్య *udata);
రకనిర్వచనం శూన్య (*duk_debug_read_flush_function) (శూన్య *udata);
రకనిర్వచనం శూన్య (*duk_debug_write_flush_function) (శూన్య *udata);
రకనిర్వచనం duk_idx_t (*duk_debug_request_function) (duk_context *ctx, శూన్య *udata, duk_idx_t nvalues);
రకనిర్వచనం శూన్య (*duk_debug_detached_function) (duk_context *ctx, శూన్య *udata);

నిర్మాణం duk_thread_state {
	/* XXX: Enough space to hold internal suspend/resume structure.
	 * This is rather awkward and to be fixed when the internal
	 * structure is visible for the public API header.
	 */
	అక్షరం దత్తాంశం[128];
};

నిర్మాణం duk_జ్ఞాపకం_functions {
	duk_alloc_function alloc_func;
	duk_realloc_function realloc_func;
	duk_free_function free_func;
	శూన్య *udata;
};

నిర్మాణం duk_function_list_నమోదు {
	స్థిరరాశి అక్షరం *కీలకం;
	duk_c_function విలువ;
	duk_idx_t nargs;
};

నిర్మాణం duk_సంఖ్య_list_నమోదు {
	స్థిరరాశి అక్షరం *కీలకం;
	duk_double_t విలువ;
};

నిర్మాణం duk_సమయం_components {
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
#నిర్వచించు DUK_DEBUG_PROTOCOL_సంస్కరణ        2

/* Used to represent invalid index; if caller uses this without checking,
 * this index will map to a non-existent stack entry.  Also used in some
 * API calls as a marker to denote "no value".
 */
#నిర్వచించు DUK_INVALID_సూచీ                 DUK_IDX_MIN

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#నిర్వచించు DUK_VARARGS                       ((duk_int_t) (-1))

/* Number of value stack entries (in addition to actual call arguments)
 * guaranteed to be allocated on entry to a Duktape/C function.
 */
#నిర్వచించు DUK_API_నమోదు_దొంతర               64U

/* Value types, used by e.g. duk_get_type() */
#నిర్వచించు DUK_TYPE_MIN                      0U
#నిర్వచించు DUK_TYPE_NONE                     0U    /* no value, e.g. invalid index */
#నిర్వచించు DUK_TYPE_UNDEFINED                1U    /* ECMAScript undefined */
#నిర్వచించు DUK_TYPE_NULL                     2U    /* ECMAScript null */
#నిర్వచించు DUK_TYPE_BOOLEAN                  3U    /* ECMAScript boolean: 0 or 1 */
#నిర్వచించు DUK_TYPE_సంఖ్య                   4U    /* ECMAScript number: double */
#నిర్వచించు DUK_TYPE_STRING                   5U    /* ECMAScript string: CESU-8 / extended UTF-8 encoded */
#నిర్వచించు DUK_TYPE_OBJECT                   6U    /* ECMAScript object: includes objects, arrays, functions, threads */
#నిర్వచించు DUK_TYPE_బఫరు                   7U    /* fixed or dynamic, garbage collected byte buffer */
#నిర్వచించు DUK_TYPE_సూచిక                  8U    /* raw void pointer */
#నిర్వచించు DUK_TYPE_LIGHTFUNC                9U    /* lightweight function pointer */
#నిర్వచించు DUK_TYPE_MAX                      9U

/* Value mask types, used by e.g. duk_get_type_mask() */
#నిర్వచించు DUK_TYPE_ముసుగు_NONE                (1U << DUK_TYPE_NONE)
#నిర్వచించు DUK_TYPE_ముసుగు_UNDEFINED           (1U << DUK_TYPE_UNDEFINED)
#నిర్వచించు DUK_TYPE_ముసుగు_NULL                (1U << DUK_TYPE_NULL)
#నిర్వచించు DUK_TYPE_ముసుగు_BOOLEAN             (1U << DUK_TYPE_BOOLEAN)
#నిర్వచించు DUK_TYPE_ముసుగు_సంఖ్య              (1U << DUK_TYPE_సంఖ్య)
#నిర్వచించు DUK_TYPE_ముసుగు_STRING              (1U << DUK_TYPE_STRING)
#నిర్వచించు DUK_TYPE_ముసుగు_OBJECT              (1U << DUK_TYPE_OBJECT)
#నిర్వచించు DUK_TYPE_ముసుగు_బఫరు              (1U << DUK_TYPE_బఫరు)
#నిర్వచించు DUK_TYPE_ముసుగు_సూచిక             (1U << DUK_TYPE_సూచిక)
#నిర్వచించు DUK_TYPE_ముసుగు_LIGHTFUNC           (1U << DUK_TYPE_LIGHTFUNC)
#నిర్వచించు DUK_TYPE_ముసుగు_THROW               (1U << 10)  /* internal flag value: throw if mask doesn't match */
#నిర్వచించు DUK_TYPE_ముసుగు_PROMOTE             (1U << 11)  /* internal flag value: promote to object if mask matches */

/* Coercion hints */
#నిర్వచించు DUK_HINT_NONE                     0    /* prefer number, unless input is a Date, in which
                                                * case prefer string (E5 Section 8.12.8)
                                                */
#నిర్వచించు DUK_HINT_STRING                   1    /* prefer string */
#నిర్వచించు DUK_HINT_సంఖ్య                   2    /* prefer number */

/* Enumeration flags for duk_enum() */
#నిర్వచించు DUK_ENUM_INCLUDE_NONENUMERABLE    (1U << 0)    /* enumerate non-numerable properties in addition to enumerable */
#నిర్వచించు DUK_ENUM_INCLUDE_HIDDEN           (1U << 1)    /* enumerate hidden symbols too (in Duktape 1.x called internal properties) */
#నిర్వచించు DUK_ENUM_INCLUDE_SYMBOLS          (1U << 2)    /* enumerate symbols */
#నిర్వచించు DUK_ENUM_EXCLUDE_అక్షరశ్రేణులు          (1U << 3)    /* exclude strings */
#నిర్వచించు DUK_ENUM_OWN_PROPERTIES_ONLY      (1U << 4)    /* don't walk prototype chain, only check own properties */
#నిర్వచించు DUK_ENUM_ARRAY_INDICES_ONLY       (1U << 5)    /* only enumerate array indices */
/* XXX: misleading name */
#నిర్వచించు DUK_ENUM_క్రమీకరించు_ARRAY_INDICES       (1U << 6)    /* sort array indices (applied to full enumeration result, including inherited array indices); XXX: misleading name */
#నిర్వచించు DUK_ENUM_NO_PROXY_BEHAVIOR        (1U << 7)    /* enumerate a proxy object itself without invoking proxy behavior */

/* Compilation flags for duk_compile() and duk_eval() */
/* DUK_COMPILE_xxx bits 0-2 are reserved for an internal 'nargs' argument.
 */
#నిర్వచించు DUK_COMPILE_EVAL                  (1U << 3)    /* compile eval code (instead of global code) */
#నిర్వచించు DUK_COMPILE_FUNCTION              (1U << 4)    /* compile function code (instead of global code) */
#నిర్వచించు DUK_COMPILE_STRICT                (1U << 5)    /* use strict (outer) context for global, eval, or function code */
#నిర్వచించు DUK_COMPILE_SHEBANG               (1U << 6)    /* allow shebang ('#! ...') comment on first line of source */
#నిర్వచించు DUK_COMPILE_సురక్షిత                  (1U << 7)    /* (internal) catch compilation errors */
#నిర్వచించు DUK_COMPILE_NORESULT              (1U << 8)    /* (internal) omit eval result */
#నిర్వచించు DUK_COMPILE_NOSOURCE              (1U << 9)    /* (internal) no source string on stack */
#నిర్వచించు DUK_COMPILE_STRLEN                (1U << 10)   /* (internal) take strlen() of src_buffer (avoids double evaluation in macro) */
#నిర్వచించు DUK_COMPILE_NOFILENAME            (1U << 11)   /* (internal) no filename on stack */
#నిర్వచించు DUK_COMPILE_FUNCEXPR              (1U << 12)   /* (internal) source is a function expression (used for Function constructor) */

/* Flags for duk_def_prop() and its variants; base flags + a lot of convenience shorthands */
#నిర్వచించు DUK_DEFPROP_WRITABLE              (1U << 0)    /* set writable (effective if DUK_DEFPROP_HAVE_WRITABLE set) */
#నిర్వచించు DUK_DEFPROP_ENUMERABLE            (1U << 1)    /* set enumerable (effective if DUK_DEFPROP_HAVE_ENUMERABLE set) */
#నిర్వచించు DUK_DEFPROP_CONFIGURABLE          (1U << 2)    /* set configurable (effective if DUK_DEFPROP_HAVE_CONFIGURABLE set) */
#నిర్వచించు DUK_DEFPROP_HAVE_WRITABLE         (1U << 3)    /* set/clear writable */
#నిర్వచించు DUK_DEFPROP_HAVE_ENUMERABLE       (1U << 4)    /* set/clear enumerable */
#నిర్వచించు DUK_DEFPROP_HAVE_CONFIGURABLE     (1U << 5)    /* set/clear configurable */
#నిర్వచించు DUK_DEFPROP_HAVE_విలువ            (1U << 6)    /* set value (given on value stack) */
#నిర్వచించు DUK_DEFPROP_HAVE_GETTER           (1U << 7)    /* set getter (given on value stack) */
#నిర్వచించు DUK_DEFPROP_HAVE_SETTER           (1U << 8)    /* set setter (given on value stack) */
#నిర్వచించు DUK_DEFPROP_FORCE                 (1U << 9)    /* force change if possible, may still fail for e.g. virtual properties */
#నిర్వచించు DUK_DEFPROP_సమితి_WRITABLE          (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_WRITABLE        DUK_DEFPROP_HAVE_WRITABLE
#నిర్వచించు DUK_DEFPROP_సమితి_ENUMERABLE        (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_ENUMERABLE      DUK_DEFPROP_HAVE_ENUMERABLE
#నిర్వచించు DUK_DEFPROP_సమితి_CONFIGURABLE      (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_CONFIGURABLE    DUK_DEFPROP_HAVE_CONFIGURABLE
#నిర్వచించు DUK_DEFPROP_W                     DUK_DEFPROP_WRITABLE
#నిర్వచించు DUK_DEFPROP_E                     DUK_DEFPROP_ENUMERABLE
#నిర్వచించు DUK_DEFPROP_C                     DUK_DEFPROP_CONFIGURABLE
#నిర్వచించు DUK_DEFPROP_WE                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE)
#నిర్వచించు DUK_DEFPROP_WC                    (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_EC                    (DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_WEC                   (DUK_DEFPROP_WRITABLE | DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_HAVE_W                DUK_DEFPROP_HAVE_WRITABLE
#నిర్వచించు DUK_DEFPROP_HAVE_E                DUK_DEFPROP_HAVE_ENUMERABLE
#నిర్వచించు DUK_DEFPROP_HAVE_C                DUK_DEFPROP_HAVE_CONFIGURABLE
#నిర్వచించు DUK_DEFPROP_HAVE_WE               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE)
#నిర్వచించు DUK_DEFPROP_HAVE_WC               (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_HAVE_EC               (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_HAVE_WEC              (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_HAVE_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_సమితి_W                 DUK_DEFPROP_సమితి_WRITABLE
#నిర్వచించు DUK_DEFPROP_సమితి_E                 DUK_DEFPROP_సమితి_ENUMERABLE
#నిర్వచించు DUK_DEFPROP_సమితి_C                 DUK_DEFPROP_సమితి_CONFIGURABLE
#నిర్వచించు DUK_DEFPROP_సమితి_WE                (DUK_DEFPROP_సమితి_WRITABLE | DUK_DEFPROP_సమితి_ENUMERABLE)
#నిర్వచించు DUK_DEFPROP_సమితి_WC                (DUK_DEFPROP_సమితి_WRITABLE | DUK_DEFPROP_సమితి_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_సమితి_EC                (DUK_DEFPROP_సమితి_ENUMERABLE | DUK_DEFPROP_సమితి_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_సమితి_WEC               (DUK_DEFPROP_సమితి_WRITABLE | DUK_DEFPROP_సమితి_ENUMERABLE | DUK_DEFPROP_సమితి_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_W               DUK_DEFPROP_CLEAR_WRITABLE
#నిర్వచించు DUK_DEFPROP_CLEAR_E               DUK_DEFPROP_CLEAR_ENUMERABLE
#నిర్వచించు DUK_DEFPROP_CLEAR_C               DUK_DEFPROP_CLEAR_CONFIGURABLE
#నిర్వచించు DUK_DEFPROP_CLEAR_WE              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_WC              (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_EC              (DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_CLEAR_WEC             (DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE | DUK_DEFPROP_CLEAR_CONFIGURABLE)
#నిర్వచించు DUK_DEFPROP_ATTR_W                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W)
#నిర్వచించు DUK_DEFPROP_ATTR_E                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E)
#నిర్వచించు DUK_DEFPROP_ATTR_C                (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C)
#నిర్వచించు DUK_DEFPROP_ATTR_WE               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE)
#నిర్వచించు DUK_DEFPROP_ATTR_WC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC)
#నిర్వచించు DUK_DEFPROP_ATTR_EC               (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_EC)
#నిర్వచించు DUK_DEFPROP_ATTR_WEC              (DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC)

/* Flags for duk_push_thread_raw() */
#నిర్వచించు DUK_THREAD_NEW_GLOBAL_ENV         (1U << 0)    /* create a new global environment */

/* Flags for duk_gc() */
#నిర్వచించు DUK_GC_COMPACT                    (1U << 0)    /* compact heap objects */

/* Error codes (must be 8 bits at most, see duk_error.h) */
#నిర్వచించు DUK_ERR_NONE                      0    /* no error (e.g. from duk_get_error_code()) */
#నిర్వచించు DUK_ERR_ERROR                     1    /* Error */
#నిర్వచించు DUK_ERR_EVAL_ERROR                2    /* EvalError */
#నిర్వచించు DUK_ERR_RANGE_ERROR               3    /* RangeError */
#నిర్వచించు DUK_ERR_REFERENCE_ERROR           4    /* ReferenceError */
#నిర్వచించు DUK_ERR_SYNTAX_ERROR              5    /* SyntaxError */
#నిర్వచించు DUK_ERR_TYPE_ERROR                6    /* TypeError */
#నిర్వచించు DUK_ERR_URI_ERROR                 7    /* URIError */

/* Return codes for C functions (shortcut for throwing an error) */
#నిర్వచించు DUK_RET_ERROR                     (-DUK_ERR_ERROR)
#నిర్వచించు DUK_RET_EVAL_ERROR                (-DUK_ERR_EVAL_ERROR)
#నిర్వచించు DUK_RET_RANGE_ERROR               (-DUK_ERR_RANGE_ERROR)
#నిర్వచించు DUK_RET_REFERENCE_ERROR           (-DUK_ERR_REFERENCE_ERROR)
#నిర్వచించు DUK_RET_SYNTAX_ERROR              (-DUK_ERR_SYNTAX_ERROR)
#నిర్వచించు DUK_RET_TYPE_ERROR                (-DUK_ERR_TYPE_ERROR)
#నిర్వచించు DUK_RET_URI_ERROR                 (-DUK_ERR_URI_ERROR)

/* Return codes for protected calls (duk_safe_call(), duk_pcall()) */
#నిర్వచించు DUK_EXEC_SUCCESS                  0
#నిర్వచించు DUK_EXEC_ERROR                    1

/* Debug levels for DUK_USE_DEBUG_WRITE(). */
#నిర్వచించు DUK_LEVEL_DEBUG                   0
#నిర్వచించు DUK_LEVEL_DDEBUG                  1
#నిర్వచించు DUK_LEVEL_DDDEBUG                 2

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

#నిర్వచించు DUK_HIDDEN_చిహ్నం(x)     ("\xFF" x)
#నిర్వచించు DUK_GLOBAL_చిహ్నం(x)     ("\x80" x)
#నిర్వచించు DUK_LOCAL_చిహ్నం(x,uniq) ("\x81" x "\xff" uniq)
#నిర్వచించు DUK_WELLKNOWN_చిహ్నం(x)  ("\x81" x "\xff")
#నిర్వచించు DUK_అంతర్గత_చిహ్నం(x)   ("\x82" x)

/*
 *  If no variadic macros, __FILE__ and __LINE__ are passed through globals
 *  which is ugly and not thread safe.
 */

#ఒకవేళ !defined(DUK_API_VARIADIC_MACROS)
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_api_global_filename;
DUK_EXTERNAL_DECL duk_int_t duk_api_global_పంక్తి;
#అయితే_ముగింపు

/*
 *  Context management
 */

DUK_EXTERNAL_DECL
duk_context *duk_create_కుప్ప(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             శూన్య *కుప్ప_udata,
                             duk_fatal_function fatal_handler);
DUK_EXTERNAL_DECL శూన్య duk_destroy_కుప్ప(duk_context *ctx);

DUK_EXTERNAL_DECL శూన్య duk_suspend(duk_context *ctx, duk_thread_state *state);
DUK_EXTERNAL_DECL శూన్య duk_resume(duk_context *ctx, స్థిరరాశి duk_thread_state *state);

#నిర్వచించు duk_create_కుప్ప_default() \
	duk_create_కుప్ప(ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ)

/*
 *  Memory management
 *
 *  Raw functions have no side effects (cannot trigger GC).
 */

DUK_EXTERNAL_DECL శూన్య *duk_alloc_raw(duk_context *ctx, duk_కొలత_t కొలత);
DUK_EXTERNAL_DECL శూన్య duk_free_raw(duk_context *ctx, శూన్య *ptr);
DUK_EXTERNAL_DECL శూన్య *duk_realloc_raw(duk_context *ctx, శూన్య *ptr, duk_కొలత_t కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_alloc(duk_context *ctx, duk_కొలత_t కొలత);
DUK_EXTERNAL_DECL శూన్య duk_free(duk_context *ctx, శూన్య *ptr);
DUK_EXTERNAL_DECL శూన్య *duk_realloc(duk_context *ctx, శూన్య *ptr, duk_కొలత_t కొలత);
DUK_EXTERNAL_DECL శూన్య duk_get_జ్ఞాపకం_functions(duk_context *ctx, duk_జ్ఞాపకం_functions *బహిర్గతం_funcs);
DUK_EXTERNAL_DECL శూన్య duk_gc(duk_context *ctx, duk_uint_t flags);

/*
 *  Error handling
 */

DUK_API_NORETURN(DUK_EXTERNAL_DECL శూన్య duk_throw_raw(duk_context *ctx));
#నిర్వచించు duk_throw(ctx) \
	(duk_throw_raw((ctx)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL శూన్య duk_fatal_raw(duk_context *ctx, స్థిరరాశి అక్షరం *err_సందేశం));
#నిర్వచించు duk_fatal(ctx,err_సందేశం) \
	(duk_fatal_raw((ctx), (err_సందేశం)), (duk_ret_t) 0)
DUK_API_NORETURN(DUK_EXTERNAL_DECL శూన్య duk_error_raw(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *filename, duk_int_t పంక్తి, స్థిరరాశి అక్షరం *fmt, ...));

#ఒకవేళ defined(DUK_API_VARIADIC_MACROS)
#నిర్వచించు duk_error(ctx,err_సంకేతం,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) (err_సంకేతం), (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_generic_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_eval_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_range_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_reference_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_syntax_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_type_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#నిర్వచించు duk_uri_error(ctx,...)  \
	(duk_error_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__), (duk_ret_t) 0)
#లేకపోతే  /* DUK_API_VARIADIC_MACROS */
/* For legacy compilers without variadic macros a macro hack is used to allow
 * variable arguments.  While the macro allows "return duk_error(...)", it
 * will fail with e.g. "(void) duk_error(...)".  The calls are noreturn but
 * with a return value to allow the "return duk_error(...)" idiom.  This may
 * cause some compiler warnings, but without noreturn the generated code is
 * often worse.  The same approach as with variadic macros (using
 * "(duk_error(...), 0)") won't work due to the macro hack structure.
 */
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_error_stash(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_generic_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_eval_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_range_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_reference_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_syntax_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_type_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
DUK_API_NORETURN(DUK_EXTERNAL_DECL duk_ret_t duk_uri_error_stash(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...));
#నిర్వచించు duk_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_error_stash)  /* last value is func pointer, arguments follow in parens */
#నిర్వచించు duk_generic_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_generic_error_stash)
#నిర్వచించు duk_eval_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_eval_error_stash)
#నిర్వచించు duk_range_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_range_error_stash)
#నిర్వచించు duk_reference_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_reference_error_stash)
#నిర్వచించు duk_syntax_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_syntax_error_stash)
#నిర్వచించు duk_type_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_type_error_stash)
#నిర్వచించు duk_uri_error  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_uri_error_stash)
#అయితే_ముగింపు  /* DUK_API_VARIADIC_MACROS */

DUK_API_NORETURN(DUK_EXTERNAL_DECL శూన్య duk_error_va_raw(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *filename, duk_int_t పంక్తి, స్థిరరాశి అక్షరం *fmt, va_list ap));

#నిర్వచించు duk_error_va(ctx,err_సంకేతం,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) (err_సంకేతం), (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_generic_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_eval_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_EVAL_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_range_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_RANGE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_reference_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_REFERENCE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_syntax_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_SYNTAX_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_type_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_TYPE_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)
#నిర్వచించు duk_uri_error_va(ctx,fmt,ap)  \
	(duk_error_va_raw((ctx), (duk_errcode_t) DUK_ERR_URI_ERROR, (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap)), (duk_ret_t) 0)

/*
 *  Other state related functions
 */

DUK_EXTERNAL_DECL duk_bool_t duk_is_strict_call(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructor_call(duk_context *ctx);

/*
 *  Stack management
 */

DUK_EXTERNAL_DECL duk_idx_t duk_normalize_సూచీ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_normalize_సూచీ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_valid_సూచీ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_require_valid_సూచీ(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_idx_t duk_get_శిఖరం(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_సమితి_శిఖరం(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_idx_t duk_get_శిఖరం_సూచీ(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_require_శిఖరం_సూచీ(duk_context *ctx);

/* Although extra/top could be an unsigned type here, using a signed type
 * makes the API more robust to calling code calculation errors or corner
 * cases (where caller might occasionally come up with negative values).
 * Negative values are treated as zero, which is better than casting them
 * to a large unsigned number.  (This principle is used elsewhere in the
 * API too.)
 */
DUK_EXTERNAL_DECL duk_bool_t duk_check_దొంతర(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL శూన్య duk_require_దొంతర(duk_context *ctx, duk_idx_t extra);
DUK_EXTERNAL_DECL duk_bool_t duk_check_దొంతర_శిఖరం(duk_context *ctx, duk_idx_t శిఖరం);
DUK_EXTERNAL_DECL శూన్య duk_require_దొంతర_శిఖరం(duk_context *ctx, duk_idx_t శిఖరం);

/*
 *  Stack manipulation (other than push/pop)
 */

DUK_EXTERNAL_DECL శూన్య duk_మార్పిడి(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2);
DUK_EXTERNAL_DECL శూన్య duk_మార్పిడి_శిఖరం(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_dup(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL శూన్య duk_dup_శిఖరం(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_చొప్పించు(duk_context *ctx, duk_idx_t గమ్యం_idx);
DUK_EXTERNAL_DECL శూన్య duk_pull(duk_context *ctx, duk_idx_t from_idx);
DUK_EXTERNAL_DECL శూన్య duk_replace(duk_context *ctx, duk_idx_t గమ్యం_idx);
DUK_EXTERNAL_DECL శూన్య duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t గమ్యం_idx);
DUK_EXTERNAL_DECL శూన్య duk_remove(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_xcopymove_raw(duk_context *గమ్యం_ctx, duk_context *from_ctx, duk_idx_t లెక్క, duk_bool_t is_copy);

#నిర్వచించు duk_xmove_శిఖరం(గమ్యం_ctx,from_ctx,లెక్క) \
	duk_xcopymove_raw((గమ్యం_ctx), (from_ctx), (లెక్క), 0 /*is_copy*/)
#నిర్వచించు duk_xcopy_శిఖరం(గమ్యం_ctx,from_ctx,లెక్క) \
	duk_xcopymove_raw((గమ్యం_ctx), (from_ctx), (లెక్క), 1 /*is_copy*/)

/*
 *  Push operations
 *
 *  Push functions return the absolute (relative to bottom of frame)
 *  position of the pushed value for convenience.
 *
 *  Note: duk_dup() is technically a push.
 */

DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_undefined(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_null(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_boolean(duk_context *ctx, duk_bool_t val);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_true(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_false(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_సంఖ్య(duk_context *ctx, duk_double_t val);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_nan(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_int(duk_context *ctx, duk_int_t val);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_uint(duk_context *ctx, duk_uint_t val);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_నెట్టు_string(duk_context *ctx, స్థిరరాశి అక్షరం *str);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_నెట్టు_lstring(duk_context *ctx, స్థిరరాశి అక్షరం *str, duk_కొలత_t len);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_సూచిక(duk_context *ctx, శూన్య *p);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_నెట్టు_sprintf(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, ...);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_నెట్టు_vsprintf(duk_context *ctx, స్థిరరాశి అక్షరం *fmt, va_list ap);

/* duk_push_literal() may evaluate its argument (a C string literal) more than
 * once on purpose.  When speed is preferred, sizeof() avoids an unnecessary
 * strlen() at runtime.  Sizeof("foo") == 4, so subtract 1.  The argument
 * must be non-NULL and should not contain internal NUL characters as the
 * behavior will then depend on config options.
 */
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_నెట్టు_literal(ctx,cstring)  duk_నెట్టు_string((ctx), (cstring))
#లేకపోతే
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_నెట్టు_literal_raw(duk_context *ctx, స్థిరరాశి అక్షరం *str, duk_కొలత_t len);
#నిర్వచించు duk_నెట్టు_literal(ctx,cstring)  duk_నెట్టు_literal_raw((ctx), (cstring), పరిమాణం((cstring)) - 1U)
#అయితే_ముగింపు

DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_this(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_new_లక్ష్యం(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_current_function(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_current_thread(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_global_object(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_కుప్ప_stash(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_global_stash(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_thread_stash(duk_context *ctx, duk_context *లక్ష్యం_ctx);

DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_bare_object(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_bare_array(duk_context *ctx);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_c_lightfunc(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_idx_t నిడివి, duk_int_t magic);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_thread_raw(duk_context *ctx, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_proxy(duk_context *ctx, duk_uint_t proxy_flags);

#నిర్వచించు duk_నెట్టు_thread(ctx) \
	duk_నెట్టు_thread_raw((ctx), 0 /*flags*/)

#నిర్వచించు duk_నెట్టు_thread_new_globalenv(ctx) \
	duk_నెట్టు_thread_raw((ctx), DUK_THREAD_NEW_GLOBAL_ENV /*flags*/)

DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_error_object_raw(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *filename, duk_int_t పంక్తి, స్థిరరాశి అక్షరం *fmt, ...);

#ఒకవేళ defined(DUK_API_VARIADIC_MACROS)
#నిర్వచించు duk_నెట్టు_error_object(ctx,err_సంకేతం,...)  \
	duk_నెట్టు_error_object_raw((ctx), (err_సంకేతం), (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), __VA_ARGS__)
#లేకపోతే
DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_error_object_stash(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *fmt, ...);
/* Note: parentheses are required so that the comma expression works in assignments. */
#నిర్వచించు duk_నెట్టు_error_object  \
	(duk_api_global_filename = (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), \
	 duk_api_global_పంక్తి = (duk_int_t) (DUK_పంక్తి_MACRO), \
	 duk_నెట్టు_error_object_stash)  /* last value is func pointer, arguments follow in parens */
#అయితే_ముగింపు

DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_error_object_va_raw(duk_context *ctx, duk_errcode_t err_సంకేతం, స్థిరరాశి అక్షరం *filename, duk_int_t పంక్తి, స్థిరరాశి అక్షరం *fmt, va_list ap);
#నిర్వచించు duk_నెట్టు_error_object_va(ctx,err_సంకేతం,fmt,ap)  \
	duk_నెట్టు_error_object_va_raw((ctx), (err_సంకేతం), (స్థిరరాశి అక్షరం *) (DUK_FILE_MACRO), (duk_int_t) (DUK_పంక్తి_MACRO), (fmt), (ap))

#నిర్వచించు DUK_BUF_FLAG_DYNAMIC   (1 << 0)    /* internal flag: dynamic buffer */
#నిర్వచించు DUK_BUF_FLAG_EXTERNAL  (1 << 1)    /* internal flag: external buffer */
#నిర్వచించు DUK_BUF_FLAG_NOZERO    (1 << 2)    /* internal flag: don't zero allocated buffer */

DUK_EXTERNAL_DECL శూన్య *duk_నెట్టు_బఫరు_raw(duk_context *ctx, duk_కొలత_t కొలత, duk_small_uint_t flags);

#నిర్వచించు duk_నెట్టు_బఫరు(ctx,కొలత,dynamic) \
	duk_నెట్టు_బఫరు_raw((ctx), (కొలత), (dynamic) ? DUK_BUF_FLAG_DYNAMIC : 0)
#నిర్వచించు duk_నెట్టు_fixed_బఫరు(ctx,కొలత) \
	duk_నెట్టు_బఫరు_raw((ctx), (కొలత), 0 /*flags*/)
#నిర్వచించు duk_నెట్టు_dynamic_బఫరు(ctx,కొలత) \
	duk_నెట్టు_బఫరు_raw((ctx), (కొలత), DUK_BUF_FLAG_DYNAMIC /*flags*/)
#నిర్వచించు duk_నెట్టు_external_బఫరు(ctx) \
	((శూన్య) duk_నెట్టు_బఫరు_raw((ctx), 0, DUK_BUF_FLAG_DYNAMIC | DUK_BUF_FLAG_EXTERNAL))

#నిర్వచించు DUK_BUFOBJ_ARRAYBUFFER         0
#నిర్వచించు DUK_BUFOBJ_NODEJS_బఫరు       1
#నిర్వచించు DUK_BUFOBJ_DATAVIEW            2
#నిర్వచించు DUK_BUFOBJ_INT8ARRAY           3
#నిర్వచించు DUK_BUFOBJ_UINT8ARRAY          4
#నిర్వచించు DUK_BUFOBJ_UINT8CLAMPEDARRAY   5
#నిర్వచించు DUK_BUFOBJ_INT16ARRAY          6
#నిర్వచించు DUK_BUFOBJ_UINT16ARRAY         7
#నిర్వచించు DUK_BUFOBJ_INT32ARRAY          8
#నిర్వచించు DUK_BUFOBJ_UINT32ARRAY         9
#నిర్వచించు DUK_BUFOBJ_FLOAT32ARRAY        10
#నిర్వచించు DUK_BUFOBJ_FLOAT64ARRAY        11

DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_బఫరు_object(duk_context *ctx, duk_idx_t idx_బఫరు, duk_కొలత_t byte_స్థానభ్రంశం, duk_కొలత_t byte_నిడివి, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_idx_t duk_నెట్టు_heapptr(duk_context *ctx, శూన్య *ptr);

/*
 *  Pop operations
 */

DUK_EXTERNAL_DECL శూన్య duk_తీయి(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_తీయి_n(duk_context *ctx, duk_idx_t లెక్క);
DUK_EXTERNAL_DECL శూన్య duk_తీయి_2(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_తీయి_3(duk_context *ctx);

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t idx, duk_int_t type);
DUK_EXTERNAL_DECL duk_uint_t duk_get_type_ముసుగు(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_check_type_ముసుగు(duk_context *ctx, duk_idx_t idx, duk_uint_t ముసుగు);

DUK_EXTERNAL_DECL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx);
#నిర్వచించు duk_is_null_or_undefined(ctx, idx) \
	((duk_get_type_ముసుగు((ctx), (idx)) & (DUK_TYPE_ముసుగు_NULL | DUK_TYPE_ముసుగు_UNDEFINED)) ? 1 : 0)

DUK_EXTERNAL_DECL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_సంఖ్య(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_బఫరు(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_బఫరు_దత్తాంశం(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_సూచిక(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_చిహ్నం(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx);

#నిర్వచించు duk_is_callable(ctx,idx) \
	duk_is_function((ctx), (idx))
DUK_EXTERNAL_DECL duk_bool_t duk_is_constructable(duk_context *ctx, duk_idx_t idx);

DUK_EXTERNAL_DECL duk_bool_t duk_is_dynamic_బఫరు(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_fixed_బఫరు(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_is_external_బఫరు(duk_context *ctx, duk_idx_t idx);

/* Buffers and lightfuncs are not considered primitive because they mimic
 * objects and e.g. duk_to_primitive() will coerce them instead of returning
 * them as is.  Symbols are represented as strings internally.
 */
#నిర్వచించు duk_is_primitive(ctx,idx) \
	duk_check_type_ముసుగు((ctx), (idx), DUK_TYPE_ముసుగు_UNDEFINED | \
	                                  DUK_TYPE_ముసుగు_NULL | \
	                                  DUK_TYPE_ముసుగు_BOOLEAN | \
	                                  DUK_TYPE_ముసుగు_సంఖ్య | \
	                                  DUK_TYPE_ముసుగు_STRING | \
	                                  DUK_TYPE_ముసుగు_సూచిక)

/* Symbols are object coercible, covered by DUK_TYPE_MASK_STRING. */
#నిర్వచించు duk_is_object_coercible(ctx,idx) \
	duk_check_type_ముసుగు((ctx), (idx), DUK_TYPE_ముసుగు_BOOLEAN | \
	                                  DUK_TYPE_ముసుగు_సంఖ్య | \
	                                  DUK_TYPE_ముసుగు_STRING | \
	                                  DUK_TYPE_ముసుగు_OBJECT | \
	                                  DUK_TYPE_ముసుగు_బఫరు | \
	                                  DUK_TYPE_ముసుగు_సూచిక | \
	                                  DUK_TYPE_ముసుగు_LIGHTFUNC)

DUK_EXTERNAL_DECL duk_errcode_t duk_get_error_సంకేతం(duk_context *ctx, duk_idx_t idx);
#నిర్వచించు duk_is_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) != 0)
#నిర్వచించు duk_is_eval_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_EVAL_ERROR)
#నిర్వచించు duk_is_range_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_RANGE_ERROR)
#నిర్వచించు duk_is_reference_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_REFERENCE_ERROR)
#నిర్వచించు duk_is_syntax_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_SYNTAX_ERROR)
#నిర్వచించు duk_is_type_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_TYPE_ERROR)
#నిర్వచించు duk_is_uri_error(ctx,idx) \
	(duk_get_error_సంకేతం((ctx), (idx)) == DUK_ERR_URI_ERROR)

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_get_సంఖ్య(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_get_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len);
DUK_EXTERNAL_DECL శూన్య *duk_get_బఫరు(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_get_బఫరు_దత్తాంశం(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_get_సూచిక(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_get_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య *duk_get_heapptr(duk_context *ctx, duk_idx_t idx);

/*
 *  Get-with-explicit default operations: like get operations but with an
 *  explicit default value.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_get_boolean_default(duk_context *ctx, duk_idx_t idx, duk_bool_t def_విలువ);
DUK_EXTERNAL_DECL duk_double_t duk_get_సంఖ్య_default(duk_context *ctx, duk_idx_t idx, duk_double_t def_విలువ);
DUK_EXTERNAL_DECL duk_int_t duk_get_int_default(duk_context *ctx, duk_idx_t idx, duk_int_t def_విలువ);
DUK_EXTERNAL_DECL duk_uint_t duk_get_uint_default(duk_context *ctx, duk_idx_t idx, duk_uint_t def_విలువ);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_get_string_default(duk_context *ctx, duk_idx_t idx, స్థిరరాశి అక్షరం *def_విలువ);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_get_lstring_default(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len, స్థిరరాశి అక్షరం *def_ptr, duk_కొలత_t def_len);
DUK_EXTERNAL_DECL శూన్య *duk_get_బఫరు_default(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత, శూన్య *def_ptr, duk_కొలత_t def_len);
DUK_EXTERNAL_DECL శూన్య *duk_get_బఫరు_దత్తాంశం_default(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత, శూన్య *def_ptr, duk_కొలత_t def_len);
DUK_EXTERNAL_DECL శూన్య *duk_get_సూచిక_default(duk_context *ctx, duk_idx_t idx, శూన్య *def_విలువ);
DUK_EXTERNAL_DECL duk_c_function duk_get_c_function_default(duk_context *ctx, duk_idx_t idx, duk_c_function def_విలువ);
DUK_EXTERNAL_DECL duk_context *duk_get_context_default(duk_context *ctx, duk_idx_t idx, duk_context *def_విలువ);
DUK_EXTERNAL_DECL శూన్య *duk_get_heapptr_default(duk_context *ctx, duk_idx_t idx, శూన్య *def_విలువ);

/*
 *  Opt operations: like require operations but with an explicit default value
 *  when value is undefined or index is invalid, null and non-matching types
 *  cause a TypeError.
 */

DUK_EXTERNAL_DECL duk_bool_t duk_opt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_విలువ);
DUK_EXTERNAL_DECL duk_double_t duk_opt_సంఖ్య(duk_context *ctx, duk_idx_t idx, duk_double_t def_విలువ);
DUK_EXTERNAL_DECL duk_int_t duk_opt_int(duk_context *ctx, duk_idx_t idx, duk_int_t def_విలువ);
DUK_EXTERNAL_DECL duk_uint_t duk_opt_uint(duk_context *ctx, duk_idx_t idx, duk_uint_t def_విలువ);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_opt_string(duk_context *ctx, duk_idx_t idx, స్థిరరాశి అక్షరం *def_ptr);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_opt_lstring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len, స్థిరరాశి అక్షరం *def_ptr, duk_కొలత_t def_len);
DUK_EXTERNAL_DECL శూన్య *duk_opt_బఫరు(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత, శూన్య *def_ptr, duk_కొలత_t def_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_opt_బఫరు_దత్తాంశం(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత, శూన్య *def_ptr, duk_కొలత_t def_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_opt_సూచిక(duk_context *ctx, duk_idx_t idx, శూన్య *def_విలువ);
DUK_EXTERNAL_DECL duk_c_function duk_opt_c_function(duk_context *ctx, duk_idx_t idx, duk_c_function def_విలువ);
DUK_EXTERNAL_DECL duk_context *duk_opt_context(duk_context *ctx, duk_idx_t idx, duk_context *def_విలువ);
DUK_EXTERNAL_DECL శూన్య *duk_opt_heapptr(duk_context *ctx, duk_idx_t idx, శూన్య *def_విలువ);

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

#నిర్వచించు duk_require_type_ముసుగు(ctx,idx,ముసుగు) \
	((శూన్య) duk_check_type_ముసుగు((ctx), (idx), (ముసుగు) | DUK_TYPE_ముసుగు_THROW))

DUK_EXTERNAL_DECL శూన్య duk_require_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_require_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_require_సంఖ్య(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_require_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len);
DUK_EXTERNAL_DECL శూన్య duk_require_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య *duk_require_బఫరు(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_require_బఫరు_దత్తాంశం(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_require_సూచిక(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_context *duk_require_context(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_require_function(duk_context *ctx, duk_idx_t idx);
#నిర్వచించు duk_require_callable(ctx,idx) \
	duk_require_function((ctx), (idx))
DUK_EXTERNAL_DECL శూన్య duk_require_constructor_call(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_require_constructable(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య *duk_require_heapptr(duk_context *ctx, duk_idx_t idx);

/* Symbols are object coercible and covered by DUK_TYPE_MASK_STRING. */
#నిర్వచించు duk_require_object_coercible(ctx,idx) \
	((శూన్య) duk_check_type_ముసుగు((ctx), (idx), DUK_TYPE_ముసుగు_BOOLEAN | \
	                                            DUK_TYPE_ముసుగు_సంఖ్య | \
	                                            DUK_TYPE_ముసుగు_STRING | \
	                                            DUK_TYPE_ముసుగు_OBJECT | \
	                                            DUK_TYPE_ముసుగు_బఫరు | \
	                                            DUK_TYPE_ముసుగు_సూచిక | \
	                                            DUK_TYPE_ముసుగు_LIGHTFUNC | \
	                                            DUK_TYPE_ముసుగు_THROW))

/*
 *  Coercion operations: in-place coercion, return coerced value where
 *  applicable.  If index is invalid, throw error.  Some coercions may
 *  throw an expected error (e.g. from a toString() or valueOf() call)
 *  or an internal error (e.g. from out of memory).
 */

DUK_EXTERNAL_DECL శూన్య duk_గమ్యం_undefined(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_గమ్యం_null(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_bool_t duk_గమ్యం_boolean(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_double_t duk_గమ్యం_సంఖ్య(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int_t duk_గమ్యం_int(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint_t duk_గమ్యం_uint(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_int32_t duk_గమ్యం_int32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint32_t duk_గమ్యం_uint32(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_uint16_t duk_గమ్యం_uint16(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_గమ్యం_string(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_గమ్యం_lstring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len);
DUK_EXTERNAL_DECL శూన్య *duk_గమ్యం_బఫరు_raw(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత, duk_uint_t flags);
DUK_EXTERNAL_DECL శూన్య *duk_గమ్యం_సూచిక(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_గమ్యం_object(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_గమ్యం_primitive(duk_context *ctx, duk_idx_t idx, duk_int_t hint);

#నిర్వచించు DUK_BUF_బహుళకం_FIXED      0   /* internal: request fixed buffer result */
#నిర్వచించు DUK_BUF_బహుళకం_DYNAMIC    1   /* internal: request dynamic buffer result */
#నిర్వచించు DUK_BUF_బహుళకం_DONTCARE   2   /* internal: don't care about fixed/dynamic nature */

#నిర్వచించు duk_గమ్యం_బఫరు(ctx,idx,బహిర్గతం_కొలత) \
	duk_గమ్యం_బఫరు_raw((ctx), (idx), (బహిర్గతం_కొలత), DUK_BUF_బహుళకం_DONTCARE)
#నిర్వచించు duk_గమ్యం_fixed_బఫరు(ctx,idx,బహిర్గతం_కొలత) \
	duk_గమ్యం_బఫరు_raw((ctx), (idx), (బహిర్గతం_కొలత), DUK_BUF_బహుళకం_FIXED)
#నిర్వచించు duk_గమ్యం_dynamic_బఫరు(ctx,idx,బహిర్గతం_కొలత) \
	duk_గమ్యం_బఫరు_raw((ctx), (idx), (బహిర్గతం_కొలత), DUK_BUF_బహుళకం_DYNAMIC)

/* safe variants of a few coercion operations */
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_సురక్షిత_గమ్యం_lstring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_len);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_గమ్యం_stacktrace(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_సురక్షిత_గమ్యం_stacktrace(duk_context *ctx, duk_idx_t idx);
#నిర్వచించు duk_సురక్షిత_గమ్యం_string(ctx,idx) \
	duk_సురక్షిత_గమ్యం_lstring((ctx), (idx), ఖాళీ)

/*
 *  Value length
 */

DUK_EXTERNAL_DECL duk_కొలత_t duk_get_నిడివి(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_సమితి_నిడివి(duk_context *ctx, duk_idx_t idx, duk_కొలత_t len);
#ఒకవేళ 0
/* duk_require_length()? */
/* duk_opt_length()? */
#అయితే_ముగింపు

/*
 *  Misc conversion
 */

DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_base64_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_base64_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_hex_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_hex_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_json_encode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_json_decode(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags);
DUK_EXTERNAL_DECL శూన్య duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags);

DUK_EXTERNAL_DECL స్థిరరాశి అక్షరం *duk_బఫరు_గమ్యం_string(duk_context *ctx, duk_idx_t idx);

/*
 *  Buffer
 */

DUK_EXTERNAL_DECL శూన్య *duk_resize_బఫరు(duk_context *ctx, duk_idx_t idx, duk_కొలత_t new_కొలత);
DUK_EXTERNAL_DECL శూన్య *duk_steal_బఫరు(duk_context *ctx, duk_idx_t idx, duk_కొలత_t *బహిర్గతం_కొలత);
DUK_EXTERNAL_DECL శూన్య duk_config_బఫరు(duk_context *ctx, duk_idx_t idx, శూన్య *ptr, duk_కొలత_t len);

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
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_string(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_get_prop_literal(ctx,obj_idx,కీలకం)  duk_get_prop_string((ctx), (obj_idx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_get_prop_literal(ctx,obj_idx,కీలకం)  duk_get_prop_literal_raw((ctx), (obj_idx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_సూచీ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_get_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, శూన్య *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_string(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_put_prop_literal(ctx,obj_idx,కీలకం)  duk_put_prop_string((ctx), (obj_idx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_put_prop_literal(ctx,obj_idx,కీలకం)  duk_put_prop_literal_raw((ctx), (obj_idx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_సూచీ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_put_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, శూన్య *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_string(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_del_prop_literal(ctx,obj_idx,కీలకం)  duk_del_prop_string((ctx), (obj_idx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_del_prop_literal(ctx,obj_idx,కీలకం)  duk_del_prop_literal_raw((ctx), (obj_idx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_సూచీ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_del_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, శూన్య *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_string(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_lstring(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_has_prop_literal(ctx,obj_idx,కీలకం)  duk_has_prop_string((ctx), (obj_idx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_literal_raw(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_has_prop_literal(ctx,obj_idx,కీలకం)  duk_has_prop_literal_raw((ctx), (obj_idx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_సూచీ(duk_context *ctx, duk_idx_t obj_idx, duk_uarridx_t arr_idx);
DUK_EXTERNAL_DECL duk_bool_t duk_has_prop_heapptr(duk_context *ctx, duk_idx_t obj_idx, శూన్య *ptr);

DUK_EXTERNAL_DECL శూన్య duk_get_prop_desc(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);
DUK_EXTERNAL_DECL శూన్య duk_def_prop(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t flags);

DUK_EXTERNAL_DECL duk_bool_t duk_get_global_string(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_lstring(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_get_global_literal(ctx,కీలకం)  duk_get_global_string((ctx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_literal_raw(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_get_global_literal(ctx,కీలకం)  duk_get_global_literal_raw((ctx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_get_global_heapptr(duk_context *ctx, శూన్య *ptr);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_string(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం);
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_lstring(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#ఒకవేళ defined(DUK_USE_PREFER_కొలత)
#నిర్వచించు duk_put_global_literal(ctx,కీలకం)  duk_put_global_string((ctx), (కీలకం))
#లేకపోతే
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_literal_raw(duk_context *ctx, స్థిరరాశి అక్షరం *కీలకం, duk_కొలత_t కీలకం_len);
#నిర్వచించు duk_put_global_literal(ctx,కీలకం)  duk_put_global_literal_raw((ctx), (కీలకం), పరిమాణం((కీలకం)) - 1U)
#అయితే_ముగింపు
DUK_EXTERNAL_DECL duk_bool_t duk_put_global_heapptr(duk_context *ctx, శూన్య *ptr);

/*
 *  Inspection
 */

DUK_EXTERNAL_DECL శూన్య duk_inspect_విలువ(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_inspect_callstack_నమోదు(duk_context *ctx, duk_int_t level);

/*
 *  Object prototype
 */

DUK_EXTERNAL_DECL శూన్య duk_get_prototype(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_సమితి_prototype(duk_context *ctx, duk_idx_t idx);

/*
 *  Object finalizer
 */

DUK_EXTERNAL_DECL శూన్య duk_get_finalizer(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_సమితి_finalizer(duk_context *ctx, duk_idx_t idx);

/*
 *  Global object
 */

DUK_EXTERNAL_DECL శూన్య duk_సమితి_global_object(duk_context *ctx);

/*
 *  Duktape/C function magic value
 */

DUK_EXTERNAL_DECL duk_int_t duk_get_magic(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL శూన్య duk_సమితి_magic(duk_context *ctx, duk_idx_t idx, duk_int_t magic);
DUK_EXTERNAL_DECL duk_int_t duk_get_current_magic(duk_context *ctx);

/*
 *  Module helpers: put multiple function or constant properties
 */

DUK_EXTERNAL_DECL శూన్య duk_put_function_list(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి duk_function_list_నమోదు *funcs);
DUK_EXTERNAL_DECL శూన్య duk_put_సంఖ్య_list(duk_context *ctx, duk_idx_t obj_idx, స్థిరరాశి duk_సంఖ్య_list_నమోదు *numbers);

/*
 *  Object operations
 */

DUK_EXTERNAL_DECL శూన్య duk_compact(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL శూన్య duk_enum(duk_context *ctx, duk_idx_t obj_idx, duk_uint_t enum_flags);
DUK_EXTERNAL_DECL duk_bool_t duk_తదుపరి(duk_context *ctx, duk_idx_t enum_idx, duk_bool_t get_విలువ);
DUK_EXTERNAL_DECL శూన్య duk_seal(duk_context *ctx, duk_idx_t obj_idx);
DUK_EXTERNAL_DECL శూన్య duk_freeze(duk_context *ctx, duk_idx_t obj_idx);

/*
 *  String manipulation
 */

DUK_EXTERNAL_DECL శూన్య duk_concat(duk_context *ctx, duk_idx_t లెక్క);
DUK_EXTERNAL_DECL శూన్య duk_join(duk_context *ctx, duk_idx_t లెక్క);
DUK_EXTERNAL_DECL శూన్య duk_decode_string(duk_context *ctx, duk_idx_t idx, duk_decode_char_function callback, శూన్య *udata);
DUK_EXTERNAL_DECL శూన్య duk_map_string(duk_context *ctx, duk_idx_t idx, duk_map_char_function callback, శూన్య *udata);
DUK_EXTERNAL_DECL శూన్య duk_substring(duk_context *ctx, duk_idx_t idx, duk_కొలత_t ప్రారంభం_char_స్థానభ్రంశం, duk_కొలత_t చివర_char_స్థానభ్రంశం);
DUK_EXTERNAL_DECL శూన్య duk_trim(duk_context *ctx, duk_idx_t idx);
DUK_EXTERNAL_DECL duk_codepoint_t duk_char_సంకేతం_at(duk_context *ctx, duk_idx_t idx, duk_కొలత_t char_స్థానభ్రంశం);

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

DUK_EXTERNAL_DECL శూన్య duk_call(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL శూన్య duk_call_పద్ధతి(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL శూన్య duk_call_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_పద్ధతి(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pcall_prop(duk_context *ctx, duk_idx_t obj_idx, duk_idx_t nargs);
DUK_EXTERNAL_DECL శూన్య duk_new(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_pnew(duk_context *ctx, duk_idx_t nargs);
DUK_EXTERNAL_DECL duk_int_t duk_సురక్షిత_call(duk_context *ctx, duk_సురక్షిత_call_function func, శూన్య *udata, duk_idx_t nargs, duk_idx_t nrets);

/*
 *  Thread management
 */

/* There are currently no native functions to yield/resume, due to the internal
 * limitations on coroutine handling.  These will be added later.
 */

/*
 *  Compilation and evaluation
 */

DUK_EXTERNAL_DECL duk_int_t duk_eval_raw(duk_context *ctx, స్థిరరాశి అక్షరం *src_బఫరు, duk_కొలత_t src_నిడివి, duk_uint_t flags);
DUK_EXTERNAL_DECL duk_int_t duk_compile_raw(duk_context *ctx, స్థిరరాశి అక్షరం *src_బఫరు, duk_కొలత_t src_నిడివి, duk_uint_t flags);

/* plain */
#నిర్వచించు duk_eval(ctx)  \
	((శూన్య) duk_eval_raw((ctx), ఖాళీ, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_eval_noresult(ctx)  \
	((శూన్య) duk_eval_raw((ctx), ఖాళీ, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval(ctx)  \
	(duk_eval_raw((ctx), ఖాళీ, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval_noresult(ctx)  \
	(duk_eval_raw((ctx), ఖాళీ, 0, 1 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_compile(ctx,flags)  \
	((శూన్య) duk_compile_raw((ctx), ఖాళీ, 0, 2 /*args*/ | (flags)))

#నిర్వచించు duk_pcompile(ctx,flags)  \
	(duk_compile_raw((ctx), ఖాళీ, 0, 2 /*args*/ | (flags) | DUK_COMPILE_సురక్షిత))

/* string */
#నిర్వచించు duk_eval_string(ctx,src)  \
	((శూన్య) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_eval_string_noresult(ctx,src)  \
	((శూన్య) duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval_string(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval_string_noresult(ctx,src)  \
	(duk_eval_raw((ctx), (src), 0, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_compile_string(ctx,flags,src)  \
	((శూన్య) duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_compile_string_filename(ctx,flags,src)  \
	((శూన్య) duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

#నిర్వచించు duk_pcompile_string(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 0 /*args*/ | (flags) | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_pcompile_string_filename(ctx,flags,src)  \
	(duk_compile_raw((ctx), (src), 0, 1 /*args*/ | (flags) | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_STRLEN))

/* lstring */
#నిర్వచించు duk_eval_lstring(ctx,buf,len)  \
	((శూన్య) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_eval_lstring_noresult(ctx,buf,len)  \
	((శూన్య) duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval_lstring(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_NOSOURCE | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_peval_lstring_noresult(ctx,buf,len)  \
	(duk_eval_raw((ctx), buf, len, 0 /*args*/ | DUK_COMPILE_EVAL | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_compile_lstring(ctx,flags,buf,len)  \
	((శూన్య) duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_compile_lstring_filename(ctx,flags,buf,len)  \
	((శూన్య) duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_NOSOURCE))

#నిర్వచించు duk_pcompile_lstring(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 0 /*args*/ | (flags) | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME))

#నిర్వచించు duk_pcompile_lstring_filename(ctx,flags,buf,len)  \
	(duk_compile_raw((ctx), buf, len, 1 /*args*/ | (flags) | DUK_COMPILE_సురక్షిత | DUK_COMPILE_NOSOURCE))

/*
 *  Bytecode load/dump
 */

DUK_EXTERNAL_DECL శూన్య duk_dump_function(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_ఎక్కించు_function(duk_context *ctx);

/*
 *  Debugging
 */

DUK_EXTERNAL_DECL శూన్య duk_నెట్టు_context_dump(duk_context *ctx);

/*
 *  Debugger (debug protocol)
 */

DUK_EXTERNAL_DECL శూన్య duk_debugger_attach(duk_context *ctx,
                                           duk_debug_read_function read_cb,
                                           duk_debug_write_function write_cb,
                                           duk_debug_peek_function peek_cb,
                                           duk_debug_read_flush_function read_flush_cb,
                                           duk_debug_write_flush_function write_flush_cb,
                                           duk_debug_request_function request_cb,
                                           duk_debug_detached_function detached_cb,
                                           శూన్య *udata);
DUK_EXTERNAL_DECL శూన్య duk_debugger_detach(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_debugger_cooperate(duk_context *ctx);
DUK_EXTERNAL_DECL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues);
DUK_EXTERNAL_DECL శూన్య duk_debugger_pause(duk_context *ctx);

/*
 *  Time handling
 */

DUK_EXTERNAL_DECL duk_double_t duk_get_now(duk_context *ctx);
DUK_EXTERNAL_DECL శూన్య duk_సమయం_గమ్యం_components(duk_context *ctx, duk_double_t timeval, duk_సమయం_components *comp);
DUK_EXTERNAL_DECL duk_double_t duk_components_గమ్యం_సమయం(duk_context *ctx, duk_సమయం_components *comp);

/*
 *  Date provider related constants
 *
 *  NOTE: These are "semi public" - you should only use these if you write
 *  your own platform specific Date provider, see doc/datetime.rst.
 */

/* Millisecond count constants. */
#నిర్వచించు DUK_DATE_MSEC_రెండవ          1000L
#నిర్వచించు DUK_DATE_MSEC_MINUTE          (60L * 1000L)
#నిర్వచించు DUK_DATE_MSEC_HOUR            (60L * 60L * 1000L)
#నిర్వచించు DUK_DATE_MSEC_DAY             (24L * 60L * 60L * 1000L)

/* ECMAScript date range is 100 million days from Epoch:
 * > 100e6 * 24 * 60 * 60 * 1000  // 100M days in millisecs
 * 8640000000000000
 * (= 8.64e15)
 */
#నిర్వచించు DUK_DATE_MSEC_100M_DAYS         (8.64e15)
#నిర్వచించు DUK_DATE_MSEC_100M_DAYS_LEEWAY  (8.64e15 + 24 * 3600e3)

/* ECMAScript year range:
 * > new Date(100e6 * 24 * 3600e3).toISOString()
 * '+275760-09-13T00:00:00.000Z'
 * > new Date(-100e6 * 24 * 3600e3).toISOString()
 * '-271821-04-20T00:00:00.000Z'
 */
#నిర్వచించు DUK_DATE_MIN_ECMA_YEAR     (-271821L)
#నిర్వచించు DUK_DATE_MAX_ECMA_YEAR     275760L

/* Part indices for internal breakdowns.  Part order from DUK_DATE_IDX_YEAR
 * to DUK_DATE_IDX_MILLISECOND matches argument ordering of ECMAScript API
 * calls (like Date constructor call).  Some functions in duk_bi_date.c
 * depend on the specific ordering, so change with care.  16 bits are not
 * enough for all parts (year, specifically).
 *
 * Must be in-sync with genbuiltins.py.
 */
#నిర్వచించు DUK_DATE_IDX_YEAR           0  /* year */
#నిర్వచించు DUK_DATE_IDX_MONTH          1  /* month: 0 to 11 */
#నిర్వచించు DUK_DATE_IDX_DAY            2  /* day within month: 0 to 30 */
#నిర్వచించు DUK_DATE_IDX_HOUR           3
#నిర్వచించు DUK_DATE_IDX_MINUTE         4
#నిర్వచించు DUK_DATE_IDX_రెండవ         5
#నిర్వచించు DUK_DATE_IDX_MILLISECOND    6
#నిర్వచించు DUK_DATE_IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#నిర్వచించు DUK_DATE_IDX_అంకె_PARTS      8

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

#నిర్వచించు DUK_DATE_FLAG_NAN_గమ్యం_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#నిర్వచించు DUK_DATE_FLAG_NAN_గమ్యం_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#నిర్వచించు DUK_DATE_FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#నిర్వచించు DUK_DATE_FLAG_EQUIVYEAR            (1 << 3)  /* timeval breakdown: replace year with equivalent year in the [1971,2037] range for DST calculations */
#నిర్వచించు DUK_DATE_FLAG_LOCALTIME            (1 << 4)  /* convert time value to local time */
#నిర్వచించు DUK_DATE_FLAG_SUB1900              (1 << 5)  /* getter: subtract 1900 from year when getting year part */
#నిర్వచించు DUK_DATE_FLAG_TOSTRING_DATE        (1 << 6)  /* include date part in string conversion result */
#నిర్వచించు DUK_DATE_FLAG_TOSTRING_సమయం        (1 << 7)  /* include time part in string conversion result */
#నిర్వచించు DUK_DATE_FLAG_TOSTRING_LOCALE      (1 << 8)  /* use locale specific formatting if available */
#నిర్వచించు DUK_DATE_FLAG_TIMESETTER           (1 << 9)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#నిర్వచించు DUK_DATE_FLAG_YEAR_FIXUP           (1 << 10) /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#నిర్వచించు DUK_DATE_FLAG_SEP_T                (1 << 11) /* string conversion: use 'T' instead of ' ' as a separator */
#నిర్వచించు DUK_DATE_FLAG_విలువ_జరుపు          12        /* additional values begin at bit 12 */

/*
 *  ROM pointer compression
 */

/* Support array for ROM pointer compression.  Only declared when ROM
 * pointer compression is active.
 */
#ఒకవేళ defined(DUK_USE_ROM_OBJECTS) && defined(DUK_USE_HEAPPTR16)
DUK_EXTERNAL_DECL స్థిరరాశి శూన్య * స్థిరరాశి duk_rom_compressed_సూచికలు[];
#అయితే_ముగింపు

/*
 *  C++ name mangling
 */

#ఒకవేళ defined(__cplusplus)
/* end 'extern "C"' wrapper */
}
#అయితే_ముగింపు

/*
 *  END PUBLIC API
 */

#అయితే_ముగింపు  /* DUKTAPE_H_INCLUDED */
